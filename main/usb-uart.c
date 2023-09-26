#include <simple-uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>
#include <esp_log.h>
#include "usb.h"
#include "usb-uart.h"
#include "network-uart.h"

#define USB_UART_PORT_NUM UART_NUM_0
#define USB_UART_TXD_PIN (43)
#define USB_UART_RXD_PIN (44)
#define USB_UART_BAUD_RATE (230400)
#define USB_UART_RX_BUF_SIZE (1024)

#define UART_RX_STREAM_BUFFER_SIZE_BYTES 1024 * 1024
static uint8_t uart_rx_stream_storage[UART_RX_STREAM_BUFFER_SIZE_BYTES + 1] EXT_RAM_ATTR;
static StaticStreamBuffer_t uart_rx_stream_buffer_struct;
static StreamBufferHandle_t uart_rx_stream = NULL;

static void usb_uart_rx_isr(void* context);
static void usb_uart_rx_task(void* pvParameters);

static const char* TAG = "usb-uart";

void usb_uart_init() {
    ESP_LOGI(TAG, "init");

    uart_rx_stream = xStreamBufferCreateStatic(
        UART_RX_STREAM_BUFFER_SIZE_BYTES, 1, uart_rx_stream_storage, &uart_rx_stream_buffer_struct);

    xTaskCreate(usb_uart_rx_task, "usb_uart_rx", 4096, NULL, 5, NULL);

    UartConfig config = {
        .uart_num = USB_UART_PORT_NUM,
        .baud_rate = USB_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .tx_pin_num = USB_UART_TXD_PIN,
        .rx_pin_num = USB_UART_RXD_PIN,
        .isr_context = uart_rx_stream,
        .rx_isr = usb_uart_rx_isr,
    };

    simple_uart_init(&config);
    ESP_LOGI(TAG, "init done");
}

void usb_uart_write(const uint8_t* data, size_t data_size) {
    simple_uart_write(USB_UART_PORT_NUM, data, data_size);
}

void usb_uart_set_line_state(bool dtr, bool rts) {
    // do nothing, we don't have rts and dtr pins
}

void usb_uart_set_line_coding(UsbUartConfig config) {
    simple_uart_set_baud_rate(USB_UART_PORT_NUM, config.bit_rate);

    // cdc.h
    // 0: 1 stop bit
    // 1: 1.5 stop bits
    // 2: 2 stop bits
    switch(config.stop_bits) {
    case 0:
        simple_uart_set_stop_bits(USB_UART_PORT_NUM, UART_STOP_BITS_1);
        break;
    case 1:
        simple_uart_set_stop_bits(USB_UART_PORT_NUM, UART_STOP_BITS_1_5);
        break;
    case 2:
        simple_uart_set_stop_bits(USB_UART_PORT_NUM, UART_STOP_BITS_2);
        break;
    default:
        break;
    }

    // cdc.h
    // 0: None
    // 1: Odd
    // 2: Even
    // 3: Mark
    // 4: Space
    switch(config.parity) {
    case 0:
        simple_uart_set_parity(USB_UART_PORT_NUM, UART_PARITY_DISABLE);
        break;
    case 1:
        simple_uart_set_parity(USB_UART_PORT_NUM, UART_PARITY_ODD);
        break;
    case 2:
        simple_uart_set_parity(USB_UART_PORT_NUM, UART_PARITY_EVEN);
        break;
    default:
        break;
    }

    // cdc.h
    // 5, 6, 7, 8 or 16
    switch(config.parity) {
    case 5:
        simple_uart_set_data_bits(USB_UART_PORT_NUM, UART_DATA_5_BITS);
        break;
    case 6:
        simple_uart_set_data_bits(USB_UART_PORT_NUM, UART_DATA_6_BITS);
        break;
    case 7:
        simple_uart_set_data_bits(USB_UART_PORT_NUM, UART_DATA_7_BITS);
        break;
    case 8:
        simple_uart_set_data_bits(USB_UART_PORT_NUM, UART_DATA_8_BITS);
        break;
    default:
        break;
    }
}

UsbUartConfig usb_uart_get_line_coding() {
    UsbUartConfig config = {
        .bit_rate = simple_uart_get_baud_rate(USB_UART_PORT_NUM),
        .stop_bits = 0,
        .parity = 0,
        .data_bits = 0,
    };

    switch(simple_uart_get_stop_bits(USB_UART_PORT_NUM)) {
    case UART_STOP_BITS_1:
        config.stop_bits = 0;
        break;
    case UART_STOP_BITS_1_5:
        config.stop_bits = 1;
        break;
    case UART_STOP_BITS_2:
        config.stop_bits = 2;
        break;
    default:
        break;
    }

    switch(simple_uart_get_parity(USB_UART_PORT_NUM)) {
    case UART_PARITY_DISABLE:
        config.parity = 0;
        break;
    case UART_PARITY_ODD:
        config.parity = 1;
        break;
    case UART_PARITY_EVEN:
        config.parity = 2;
        break;
    default:
        break;
    }

    switch(simple_uart_get_data_bits(USB_UART_PORT_NUM)) {
    case UART_DATA_5_BITS:
        config.data_bits = 5;
        break;
    case UART_DATA_6_BITS:
        config.data_bits = 6;
        break;
    case UART_DATA_7_BITS:
        config.data_bits = 7;
        break;
    case UART_DATA_8_BITS:
        config.data_bits = 8;
        break;
    default:
        break;
    }

    return config;
}

#include "network-http.h"
static void usb_uart_rx_task(void* pvParameters) {
    uint8_t* data = malloc(USB_UART_RX_BUF_SIZE);

    while(1) {
        size_t length =
            xStreamBufferReceive(uart_rx_stream, data, USB_UART_RX_BUF_SIZE, portMAX_DELAY);

        if(length > 0) {
            for(size_t i = 0; i < length; i++) {
                if((i + 1) == length) {
                    usb_uart_tx_char(data[i], true);
                } else {
                    usb_uart_tx_char(data[i], false);
                }
            }
            network_http_uart_write_data(data, length);
            if(network_uart_connected()) {
                network_uart_send(data, length);
            }
        }
    }
}

static void usb_uart_rx_isr(void* context) {
    StreamBufferHandle_t stream = context;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint8_t data;
    while(simple_uart_available(USB_UART_PORT_NUM)) {
        simple_uart_read(USB_UART_PORT_NUM, &data, 1);

        size_t ret __attribute__((unused));
        // we will drop data if the stream overflows
        ret = xStreamBufferSendFromISR(stream, &data, 1, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}