/**
 * @file usb-cdc.c
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
#include "usb-cdc.h"
#include "usb-uart.h"
#include "led-dotstar.h"
#include "delay.h"
#include <gdb-glue.h>
#include <dual-cdc-driver.h>
#include <class/cdc/cdc_device.h>

#define USB_DN_PIN (19)
#define USB_DP_PIN (20)

#define GDB_BUF_RX_SIZE 64
#define UART_BUF_RX_SIZE 64

static const char* TAG = "usb-cdc";
static uint8_t gdb_buffer_rx[GDB_BUF_RX_SIZE];
static uint8_t uart_buffer_rx[UART_BUF_RX_SIZE];

typedef struct {
    volatile bool connected;
} USBCDC;

static USBCDC usb_cdc;

typedef enum {
    CDCTypeGDB = 0,
    CDCTypeUART = 1,
} CDCType;

static void usb_cdc_tx_char(CDCType type, uint8_t c, bool flush) {
    tud_cdc_n_write(type, &c, 1);

    if(flush) {
        tud_cdc_n_write_flush(type);
    }
}

void usb_cdc_gdb_tx_char(uint8_t c, bool flush) {
    usb_cdc_tx_char(CDCTypeGDB, c, flush);
}

void usb_cdc_uart_tx_char(uint8_t c, bool flush) {
    usb_cdc_tx_char(CDCTypeUART, c, flush);
}

void usb_cdc_gdb_rx_callback(void) {
    if(gdb_glue_can_receive()) {
        size_t max_len = gdb_glue_get_free_size();
        if(max_len > GDB_BUF_RX_SIZE) max_len = GDB_BUF_RX_SIZE;
        uint32_t rx_size = tud_cdc_n_read(CDCTypeGDB, gdb_buffer_rx, max_len);

        if(rx_size > 0) {
            gdb_glue_receive(gdb_buffer_rx, rx_size);
        }
    } else {
        esp_system_abort("No free space in GDB buffer");
    }
}

void usb_cdc_uart_rx_callback(void) {
    size_t max_len = gdb_glue_get_free_size();
    if(max_len > UART_BUF_RX_SIZE) max_len = UART_BUF_RX_SIZE;
    uint32_t rx_size = tud_cdc_n_read(CDCTypeUART, uart_buffer_rx, max_len);

    if(rx_size > 0) {
        usb_uart_write(uart_buffer_rx, rx_size);
    }
}

void tud_cdc_rx_cb(uint8_t interface) {
    do {
        if(interface == CDCTypeGDB) {
            usb_cdc_gdb_rx_callback();
        } else if(interface == CDCTypeUART) {
            usb_cdc_uart_rx_callback();
        } else {
            tud_cdc_n_read_flush(interface);
        }
    } while(false);
}

void tud_cdc_line_state_cb(uint8_t interface, bool dtr, bool rts) {
    if(interface == CDCTypeUART) {
        usb_uart_set_line_state(dtr, rts);
    }
}

void tud_cdc_line_coding_cb(uint8_t interface, cdc_line_coding_t const* p_line_coding) {
    uint32_t bit_rate = p_line_coding->bit_rate;
    uint8_t stop_bits = p_line_coding->stop_bits;
    uint8_t parity = p_line_coding->parity;
    uint8_t data_bits = p_line_coding->data_bits;

    if(interface == CDCTypeUART) {
        usb_uart_set_line_coding(bit_rate, stop_bits, parity, data_bits);
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

static void usb_cdc_event_blink(void) {
    led_set_blue(255);
    delay(10);
    led_set_blue(0);
}

static void usb_cdc_to_connected(void) {
    if(!usb_cdc.connected) {
        usb_cdc_event_blink();
    }
    usb_cdc.connected = true;
    ESP_LOGI(TAG, "connect");
}

static void usb_cdc_from_connected(void) {
    if(usb_cdc.connected) {
        usb_cdc_event_blink();
    }

    usb_cdc.connected = false;
    ESP_LOGI(TAG, "disconnect");
}

void tud_mount_cb(void) {
    usb_cdc_to_connected();
}

void tud_umount_cb(void) {
    usb_cdc_from_connected();
}

void tud_resume_cb(void) {
    usb_cdc_to_connected();
}

void tud_suspend_cb(bool remote_wakeup_en) {
    usb_cdc_from_connected();
}

static void usb_cdc_bus_reset() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = ((1 << USB_DN_PIN) | (1 << USB_DP_PIN));
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(USB_DN_PIN, 0);
    gpio_set_level(USB_DP_PIN, 0);
    delay(100);
    gpio_set_level(USB_DN_PIN, 1);
    gpio_set_level(USB_DP_PIN, 1);
}

void usb_cdc_init(void) {
    ESP_LOGI(TAG, "init");

    usb_cdc.connected = false;
    usb_uart_init();
    usb_cdc_bus_reset();
    dual_cdc_driver_install();
    
    ESP_LOGI(TAG, "init done");
}
