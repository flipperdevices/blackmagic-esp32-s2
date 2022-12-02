/**
 * @file usb.c
 * Do not forget to take pid's when implement own USB device class
 * 
 * https://github.com/espressif/usb-pids
 * 
 */

#include <stdint.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include "usb.h"
#include "usb-uart.h"
#include "led.h"
#include "delay.h"
#include "nvs-config.h"
#include <gdb-glue.h>
#include <usb-glue.h>
#include <class/cdc/cdc_device.h>

#define USB_DN_PIN (19)
#define USB_DP_PIN (20)

#define GDB_BUF_RX_SIZE 64
#define UART_BUF_RX_SIZE 64

static const char* TAG = "usb";
static uint8_t gdb_buffer_rx[GDB_BUF_RX_SIZE];
static uint8_t uart_buffer_rx[UART_BUF_RX_SIZE];

typedef struct {
    volatile bool connected;
} UsbState;

static UsbState usb_state;

void usb_gdb_tx_char(uint8_t c, bool flush) {
    usb_glue_gdb_send(&c, 1, flush);
}

void usb_uart_tx_char(uint8_t c, bool flush) {
    usb_glue_cdc_send(&c, 1, flush);
}

static void usb_gdb_rx_callback(void* context) {
    if(gdb_glue_can_receive()) {
        size_t max_len = gdb_glue_get_free_size();
        if(max_len > GDB_BUF_RX_SIZE) max_len = GDB_BUF_RX_SIZE;
        uint32_t rx_size = usb_glue_gdb_receive(gdb_buffer_rx, max_len);

        if(rx_size > 0) {
            gdb_glue_receive(gdb_buffer_rx, rx_size);
        }
    } else {
        esp_system_abort("No free space in GDB buffer");
    }
}

static void usb_uart_rx_callback(void* context) {
    size_t max_len = gdb_glue_get_free_size();
    if(max_len > UART_BUF_RX_SIZE) max_len = UART_BUF_RX_SIZE;
    uint32_t rx_size = usb_glue_cdc_receive(uart_buffer_rx, max_len);

    if(rx_size > 0) {
        usb_uart_write(uart_buffer_rx, rx_size);
    }
}

static void usb_line_state_cb(bool dtr, bool rts, void* context) {
    usb_uart_set_line_state(dtr, rts);
}

static void usb_set_line_coding_callback(cdc_line_coding_t const* p_line_coding, void* context) {
    uint32_t bit_rate = p_line_coding->bit_rate;
    uint8_t stop_bits = p_line_coding->stop_bits;
    uint8_t parity = p_line_coding->parity;
    uint8_t data_bits = p_line_coding->data_bits;

    usb_uart_set_line_coding(bit_rate, stop_bits, parity, data_bits);
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

static void usb_event_blink(void) {
    led_set_blue(255);
    delay(10);
    led_set_blue(0);
}

static void usb_to_connected(void* context) {
    if(!usb_state.connected) {
        usb_event_blink();
    }
    usb_state.connected = true;
    ESP_LOGI(TAG, "connect");
}

static void usb_from_connected(void* context) {
    if(usb_state.connected) {
        usb_event_blink();
    }

    usb_state.connected = false;
    ESP_LOGI(TAG, "disconnect");
}

#define CONFIG_DAP_TASK_STACK_SIZE 4096
#define CONFIG_DAP_TASK_PRIORITY 5
#define DAP_RECEIVE_FLAG (1 << 0)
#define DAP_TAG "dap_task"

#include "dap.h"
#include "dap_config.h"

TaskHandle_t dap_task_handle;

static void dap_rx_callback(void* context) {
    xTaskNotify(dap_task_handle, DAP_RECEIVE_FLAG, eSetBits);
}

void dap_callback_connect(void) {
    ESP_LOGI(DAP_TAG, "connected");
}

void dap_callback_disconnect(void) {
    ESP_LOGI(DAP_TAG, "disconnected");
}

static void dap_task(void* arg) {
    ESP_LOGI(DAP_TAG, "started");
    uint32_t notified_value;
    dap_init();

    while(1) {
        BaseType_t xResult = xTaskNotifyWait(pdFALSE, ULONG_MAX, &notified_value, portMAX_DELAY);

        if(xResult == pdPASS) {
            if((notified_value & DAP_RECEIVE_FLAG) != 0) {
                uint8_t rx_data[DAP_CONFIG_PACKET_SIZE];
                uint8_t tx_data[DAP_CONFIG_PACKET_SIZE];
                memset(tx_data, 0, DAP_CONFIG_PACKET_SIZE);
                memset(rx_data, 0, DAP_CONFIG_PACKET_SIZE);

                size_t rx_size = usb_glue_dap_receive(rx_data, sizeof(rx_data));
                size_t tx_size = dap_process_request(rx_data, rx_size, tx_data, sizeof(tx_data));
                usb_glue_dap_send(tx_data, tx_size, true);
            }
        }
    }
}

static void usb_dap_init() {
    ESP_LOGI(DAP_TAG, "init");
    xTaskCreate(
        dap_task,
        "DAP",
        CONFIG_DAP_TASK_STACK_SIZE,
        NULL,
        CONFIG_DAP_TASK_PRIORITY,
        &dap_task_handle);
    ESP_LOGI(DAP_TAG, "init done");
}

void usb_init(void) {
    ESP_LOGI(TAG, "init");

    // TODO get from config
    UsbMode usb_mode = UsbModeDAP;

    if(usb_mode == UsbModeBM) {
        usb_glue_set_connected_callback(usb_to_connected, NULL);
        usb_glue_set_disconnected_callback(usb_from_connected, NULL);
        usb_glue_cdc_set_line_coding_callback(usb_set_line_coding_callback, NULL);
        usb_glue_cdc_set_line_state_callback(usb_line_state_cb, NULL);
        usb_glue_cdc_set_receive_callback(usb_uart_rx_callback, NULL);
        usb_glue_gdb_set_receive_callback(usb_gdb_rx_callback, NULL);

        usb_state.connected = false;
        usb_uart_init();
        usb_glue_init(USBDeviceTypeDualCDC);
    } else {
        usb_glue_set_connected_callback(usb_to_connected, NULL);
        usb_glue_set_disconnected_callback(usb_from_connected, NULL);
        usb_glue_cdc_set_line_coding_callback(usb_set_line_coding_callback, NULL);
        usb_glue_cdc_set_line_state_callback(usb_line_state_cb, NULL);
        usb_glue_cdc_set_receive_callback(usb_uart_rx_callback, NULL);
        usb_glue_dap_set_receive_callback(dap_rx_callback, NULL);

        usb_state.connected = false;
        usb_uart_init();
        usb_dap_init();
        usb_glue_init(USBDeviceTypeDapLink);
    }

    ESP_LOGI(TAG, "init done");
}
