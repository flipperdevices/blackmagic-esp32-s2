// #include <driver/uart.h>
// #include <driver/gpio.h>
#include <string.h>
#include "led-dotstar.h"
#include "cli/cli.h"
#include <simple-uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>

#define CLI_UART_PORT_NUM UART_NUM_1
#define CLI_UART_TXD_PIN (4)
#define CLI_UART_RXD_PIN (14)
#define CLI_UART_BAUD_RATE (115200)
#define CLI_UART_TX_BUF_SIZE (64)
#define CLI_UART_RX_BUF_SIZE (64)

static Cli* cli_uart;
static uint8_t uart_tx_buffer[CLI_UART_TX_BUF_SIZE];
static size_t uart_tx_index = 0;
static StreamBufferHandle_t uart_rx_stream;

static void cli_uart_write(const uint8_t* data, size_t data_size, void* context);
static void cli_uart_flush(void* context);

static void cli_uart_rx_isr(void* context);

static void cli_uart_rx_task(void* pvParameters) {
    while(1) {
        uint8_t data[CLI_UART_RX_BUF_SIZE];
        size_t length =
            xStreamBufferReceive(uart_rx_stream, data, CLI_UART_RX_BUF_SIZE, portMAX_DELAY);
        if(length > 0) {
            for(size_t i = 0; i < length; i++) {
                cli_handle_char(cli_uart, data[i]);
            }
        }
    }
}

void cli_uart_init() {
    cli_uart = cli_init();
    cli_set_write_cb(cli_uart, cli_uart_write);
    cli_set_flush_cb(cli_uart, cli_uart_flush);

    uart_rx_stream = xStreamBufferCreate(CLI_UART_RX_BUF_SIZE * 4, 1);

    xTaskCreate(cli_uart_rx_task, "cli_uart_rx", 4096, NULL, 5, NULL);

    UartConfig config = {
        .uart_num = CLI_UART_PORT_NUM,
        .baud_rate = CLI_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .tx_pin_num = CLI_UART_TXD_PIN,
        .rx_pin_num = CLI_UART_RXD_PIN,
        .isr_context = uart_rx_stream,
        .rx_isr = cli_uart_rx_isr,
    };

    simple_uart_init(&config);

    cli_force_motd(cli_uart);
}

static void cli_uart_write(const uint8_t* data, size_t data_size, void* context) {
    for(size_t i = 0; i < data_size; i++) {
        uart_tx_buffer[uart_tx_index] = data[i];
        uart_tx_index++;

        if(uart_tx_index == CLI_UART_TX_BUF_SIZE) {
            cli_uart_flush(NULL);
        }
    }
}

static void cli_uart_flush(void* context) {
    if(uart_tx_index > 0) {
        simple_uart_write(CLI_UART_PORT_NUM, uart_tx_buffer, uart_tx_index);
    }

    uart_tx_index = 0;
}

static void cli_uart_rx_isr(void* context) {
    StreamBufferHandle_t stream = context;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint8_t data;
    while(simple_uart_available(CLI_UART_PORT_NUM)) {
        simple_uart_read(CLI_UART_PORT_NUM, &data, 1);

        size_t ret __attribute__((unused));
        ret = xStreamBufferSendFromISR(stream, &data, 1, &xHigherPriorityTaskWoken);
        // we will drop data if the stream overflows
        // ESP_ERROR_CHECK(ret != 1);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}