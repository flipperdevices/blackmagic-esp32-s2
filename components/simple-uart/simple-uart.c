#include <driver/gpio.h>
#include <driver/periph_ctrl.h>
#include <hal/uart_ll.h>
#include <hal/uart_hal.h>

#include <hal/gpio_hal.h>
#include <esp_rom_gpio.h>

#include "simple-uart.h"

/***********************************************/

typedef struct {
    uart_hal_context_t hal; /*!< UART hal context*/
    uint8_t uart_index;
    void* isr_context;
    uart_isr rx_isr;
} uart_context_t;

typedef struct {
    uint32_t baud_rate;
    uart_stop_bits_t stop_bits;
    uart_parity_t parity;
    uart_word_length_t data_bits;
} UartInnerConfig;

#define UART_CONTEX_INIT_DEF(uart_num)                                                    \
    {                                                                                     \
        .hal.dev = UART_LL_GET_HW(uart_num), .uart_index = uart_num, .isr_context = NULL, \
        .rx_isr = NULL                                                                    \
    }

static uart_context_t uart_context[UART_NUM_MAX] = {
    UART_CONTEX_INIT_DEF(UART_NUM_0),
    UART_CONTEX_INIT_DEF(UART_NUM_1),
#if UART_NUM_MAX > 2
    UART_CONTEX_INIT_DEF(UART_NUM_2),
#endif
};

static UartInnerConfig uart_config[UART_NUM_MAX] = {
    {0},
    {0},
};

#define UART_HAL(uart_num) &(uart_context[uart_num].hal)

/***********************************************/

#define UART_FIFO_LIMIT (UART_LL_FIFO_DEF_LEN)
#define UART_TX_FIFO_THRESH 0x1
#define UART_RX_FIFO_THRESH 0x16

static void simple_uart_isr(void* arg);

static void simple_uart_init_pins(uint8_t uart_num, int tx_pin_num, int rx_pin_num) {
    if(tx_pin_num >= 0) {
        gpio_hal_iomux_func_sel(GPIO_PIN_MUX_REG[tx_pin_num], PIN_FUNC_GPIO);
        gpio_set_level(tx_pin_num, 1);
        esp_rom_gpio_connect_out_signal(
            tx_pin_num, UART_PERIPH_SIGNAL(uart_num, SOC_UART_TX_PIN_IDX), 0, 0);
    }

    if(rx_pin_num >= 0) {
        gpio_hal_iomux_func_sel(GPIO_PIN_MUX_REG[rx_pin_num], PIN_FUNC_GPIO);
        gpio_set_pull_mode(rx_pin_num, GPIO_PULLUP_ONLY);
        gpio_set_direction(rx_pin_num, GPIO_MODE_INPUT);
        esp_rom_gpio_connect_in_signal(
            rx_pin_num, UART_PERIPH_SIGNAL(uart_num, SOC_UART_RX_PIN_IDX), 0);
    }
}

static void simple_uart_init_module(uint8_t uart_num) {
    periph_module_enable(uart_periph_signal[uart_num].module);
    periph_module_reset(uart_periph_signal[uart_num].module);
}

void simple_uart_init(UartConfig* cfg) {
    uart_context[cfg->uart_num].rx_isr = cfg->rx_isr;
    uart_context[cfg->uart_num].isr_context = cfg->isr_context;

    simple_uart_init_pins(cfg->uart_num, cfg->tx_pin_num, cfg->rx_pin_num);
    simple_uart_init_module(cfg->uart_num);

    uart_hal_set_sclk(UART_HAL(cfg->uart_num), UART_SCLK_APB);
    uart_hal_set_rxfifo_full_thr(UART_HAL(cfg->uart_num), UART_RX_FIFO_THRESH);
    uart_hal_set_txfifo_empty_thr(UART_HAL(cfg->uart_num), UART_TX_FIFO_THRESH);
    uart_hal_rxfifo_rst(UART_HAL(cfg->uart_num));
    uart_hal_set_hw_flow_ctrl(UART_HAL(cfg->uart_num), UART_HW_FLOWCTRL_DISABLE, 0);
    uart_hal_set_rx_timeout(UART_HAL(cfg->uart_num), 0x16);

    simple_uart_set_baud_rate(cfg->uart_num, cfg->baud_rate);
    simple_uart_set_stop_bits(cfg->uart_num, cfg->stop_bits);
    simple_uart_set_parity(cfg->uart_num, cfg->parity);
    simple_uart_set_data_bits(cfg->uart_num, cfg->data_bits);

    esp_intr_alloc(
        uart_periph_signal[cfg->uart_num].irq,
        0,
        simple_uart_isr,
        &uart_context[cfg->uart_num],
        NULL);

    // disable interrupts
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_RXFIFO_FULL);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_TXFIFO_EMPTY);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_PARITY_ERR);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_FRAM_ERR);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_RXFIFO_OVF);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_DSR_CHG);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_CTS_CHG);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_BRK_DET);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_RXFIFO_TOUT);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_SW_XON);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_SW_XOFF);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_GLITCH_DET);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_TX_BRK_DONE);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_TX_BRK_IDLE);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_TX_DONE);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_RS485_PARITY_ERR);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_RS485_FRM_ERR);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_RS485_CLASH);
    uart_hal_clr_intsts_mask(UART_HAL(cfg->uart_num), UART_INTR_CMD_CHAR_DET);

    // enable rx interrupts
    uart_hal_ena_intr_mask(UART_HAL(cfg->uart_num), UART_INTR_RXFIFO_FULL);
    uart_hal_ena_intr_mask(UART_HAL(cfg->uart_num), UART_INTR_RXFIFO_TOUT);
}

void simple_uart_write(uint8_t uart_num, const uint8_t* data, const uint32_t data_size) {
    uint32_t to_write = data_size;
    while(to_write > 0) {
        while(uart_hal_get_txfifo_len(UART_HAL(uart_num)) == 0) {
            ; /* Wait */
        }

        uint32_t write_size = 0;
        uart_hal_write_txfifo(
            UART_HAL(uart_num), data + (data_size - to_write), to_write, &write_size);
        to_write -= write_size;
    }
}

bool simple_uart_available(uint8_t uart_num) {
    const int num_rx = uart_hal_get_rxfifo_len(UART_HAL(uart_num));
    return num_rx > 0;
}

uint32_t simple_uart_read(uint8_t uart_num, uint8_t* data, const uint32_t data_size) {
    const int num_rx = uart_hal_get_rxfifo_len(UART_HAL(uart_num));
    int read = ((data_size) < (num_rx) ? (data_size) : (num_rx));

    if(!read) {
        return 0;
    }

    uart_hal_read_rxfifo(UART_HAL(uart_num), data, &read);
    return read;
}

static void simple_uart_isr(void* arg) {
    uart_context_t* context = arg;
    uart_hal_context_t* hal_context = &context->hal;
    uint8_t hal_num = context->uart_index;

    uint32_t uart_intr_status = uart_hal_get_intsts_mask(hal_context);
    if(uart_intr_status == 0) {
        return;
    }
    uart_hal_clr_intsts_mask(hal_context, uart_intr_status);

    if(uart_intr_status & UART_INTR_TXFIFO_EMPTY) {
    }
    if((uart_intr_status & UART_INTR_RXFIFO_TOUT) || (uart_intr_status & UART_INTR_RXFIFO_FULL)) {
        if(context->rx_isr) {
            context->rx_isr(context->isr_context);
        } else {
            // purge rx fifo
            uint8_t data;
            while(simple_uart_available(hal_num)) {
                simple_uart_read(hal_num, &data, 1);
            }
        }
    }
}

void simple_uart_set_baud_rate(uint8_t uart_num, uint32_t baud_rate) {
    uart_config[uart_num].baud_rate = baud_rate;
    uart_hal_set_baudrate(UART_HAL(uart_num), baud_rate);
}

void simple_uart_set_stop_bits(uint8_t uart_num, uart_stop_bits_t stop_bits) {
    uart_config[uart_num].stop_bits = stop_bits;
    uart_hal_set_stop_bits(UART_HAL(uart_num), stop_bits);
}

void simple_uart_set_parity(uint8_t uart_num, uart_parity_t parity) {
    uart_config[uart_num].parity = parity;
    uart_hal_set_parity(UART_HAL(uart_num), parity);
}

void simple_uart_set_data_bits(uint8_t uart_num, uart_word_length_t data_bits) {
    uart_config[uart_num].data_bits = data_bits;
    uart_hal_set_data_bit_num(UART_HAL(uart_num), data_bits);
}

uint32_t simple_uart_get_baud_rate(uint8_t uart_num) {
    return uart_config[uart_num].baud_rate;
}

uart_stop_bits_t simple_uart_get_stop_bits(uint8_t uart_num) {
    return uart_config[uart_num].stop_bits;
}

uart_parity_t simple_uart_get_parity(uint8_t uart_num) {
    return uart_config[uart_num].parity;
}

uart_word_length_t simple_uart_get_data_bits(uint8_t uart_num) {
    return uart_config[uart_num].data_bits;
}