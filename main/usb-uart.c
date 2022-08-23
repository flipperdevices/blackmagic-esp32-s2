#include <simple-uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>
#include <esp_log.h>
// #include "usb-cdc.h"
#include "usb-uart.h"

#define USB_UART_PORT_NUM UART_NUM_0
#define USB_UART_TXD_PIN (1)
#define USB_UART_RXD_PIN (3)
#define USB_UART_BAUD_RATE (115200)
#define USB_UART_BUF_SIZE (128)
#define USB_UART_TX_BUF_SIZE (64)
#define USB_UART_RX_BUF_SIZE (64)

static StreamBufferHandle_t uart_rx_stream;

static void usb_uart_rx_isr(void* context);
static void usb_uart_rx_task(void* pvParameters);

static const char* TAG = "usb-uart";

void usb_uart_init() {
    ESP_LOGI(TAG, "init");

    uart_rx_stream = xStreamBufferCreate(USB_UART_BUF_SIZE * 4, 1);

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

    usb_uart_write((const uint8_t*)"Go", 2);

    ESP_LOGI(TAG, "init done");
}

void usb_uart_write(const uint8_t* data, size_t data_size) {
    simple_uart_write(USB_UART_PORT_NUM, data, data_size);
}

void usb_uart_set_line_state(bool dtr, bool rts) {
    // do nothing, we don't have rts and dtr pins
}

void usb_uart_set_line_coding(
    uint32_t bit_rate,
    uint8_t stop_bits,
    uint8_t parity,
    uint8_t data_bits) {
    simple_uart_set_baud_rate(USB_UART_PORT_NUM, bit_rate);

    // cdc.h
    // 0: 1 stop bit
    // 1: 1.5 stop bits
    // 2: 2 stop bits
    switch(stop_bits) {
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
    switch(parity) {
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
    switch(parity) {
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

static void usb_uart_rx_task(void* pvParameters) {
    while(1) {
        uint8_t data[USB_UART_RX_BUF_SIZE];
        size_t length =
            xStreamBufferReceive(uart_rx_stream, data, USB_UART_RX_BUF_SIZE, portMAX_DELAY);

        if(length > 0) {
            for(size_t i = 0; i < length; i++) {
                if((i + 1) == length) {
                    usb_cdc_uart_tx_char(data[i], true);
                } else {
                    usb_cdc_uart_tx_char(data[i], false);
                }
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