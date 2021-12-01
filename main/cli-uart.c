#include <driver/uart.h>
#include <driver/gpio.h>
#include <string.h>
#include "cli.h"

#define CLI_UART_PORT_NUM UART_NUM_1
#define CLI_UART_TXD_PIN (17)
#define CLI_UART_RXD_PIN (18)
#define CLI_UART_RTS_PIN (UART_PIN_NO_CHANGE)
#define CLI_UART_CTS_PIN (UART_PIN_NO_CHANGE)
#define CLI_UART_BAUD_RATE (115200)
#define CLI_UART_BUF_SIZE (128)

static Cli* cli_uart;
static QueueHandle_t cli_uart_queue;
static void cli_uart_write(const uint8_t* data, size_t data_size, void* context);

static void cli_uart_rx_task(void* pvParameters) {
    uart_event_t event;
    uint8_t* rx_buffer = (uint8_t*)malloc(CLI_UART_BUF_SIZE);
    int received = 0;

    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(cli_uart_queue, (void*)&event, (portTickType)portMAX_DELAY)) {
            bzero(rx_buffer, CLI_UART_BUF_SIZE);
            switch(event.type) {
            case UART_DATA:
                received =
                    uart_read_bytes(CLI_UART_PORT_NUM, rx_buffer, event.size, portMAX_DELAY);
                for(int i = 0; i < received; i++) {
                    cli_handle_char(cli_uart, rx_buffer[i]);
                }

                break;
            default:
                break;
            }
        }
    }

    free(rx_buffer);
    rx_buffer = NULL;
    vTaskDelete(NULL);
}

void cli_uart_init() {
    cli_uart = cli_init();
    cli_set_write_cb(cli_uart, cli_uart_write);

    uart_config_t uart_config = {
        .baud_rate = CLI_UART_BAUD_RATE,
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

    ESP_ERROR_CHECK(uart_driver_install(
        CLI_UART_PORT_NUM,
        CLI_UART_BUF_SIZE * 2,
        CLI_UART_BUF_SIZE * 2,
        10,
        &cli_uart_queue,
        intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(CLI_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(
        CLI_UART_PORT_NUM, CLI_UART_TXD_PIN, CLI_UART_RXD_PIN, CLI_UART_RTS_PIN, CLI_UART_CTS_PIN));

    xTaskCreate(cli_uart_rx_task, "cli_uart_rx", 4096, NULL, 5, NULL);
}

static void cli_uart_write(const uint8_t* data, size_t data_size, void* context) {
    uart_write_bytes(CLI_UART_PORT_NUM, data, data_size);
}
