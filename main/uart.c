#include <driver/uart.h>
#include <driver/gpio.h>
#include <string.h>

#define UART_PORT_NUM UART_NUM_1
#define UART_TXD_PIN (17)
#define UART_RXD_PIN (18)
#define UART_RTS_PIN (UART_PIN_NO_CHANGE)
#define UART_CTS_PIN (UART_PIN_NO_CHANGE)
#define UART_BAUD_RATE (115200)
#define UART_BUF_SIZE (1024)

void uart_init() {
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(
        uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(
        uart_set_pin(UART_PORT_NUM, UART_TXD_PIN, UART_RXD_PIN, UART_RTS_PIN, UART_CTS_PIN));
}

void uart_print(const char* str) {
    uart_write_bytes(UART_PORT_NUM, str, strlen(str));
}
