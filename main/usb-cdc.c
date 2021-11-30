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
#include <tinyusb.h>
#include <tusb_cdc_acm.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include "usb-cdc.h"
#include "led.h"
#include "delay.h"
#include <gdb-glue.h>

#define USB_DN_PIN (19)
#define USB_DP_PIN (20)
#define CDC_USB_DEV (TINYUSB_USBDEV_0)

static const char* TAG = "usb-cdc";
static uint8_t buffer_rx[CONFIG_USB_CDC_RX_BUFSIZE];

typedef struct {
    volatile bool connected;
    volatile bool dtr;
    volatile bool rts;
} USBCDC;

static USBCDC usb_cdc;

void usb_cdc_tx_char(uint8_t c, bool flush) {
    tinyusb_cdcacm_write_queue(CDC_USB_DEV, &c, 1);

    if(flush) {
        // SOME GDB MAGIC
        // We need to send an empty packet for some hosts to accept this as a complete transfer.
        uint8_t zero_byte = 0;
        tinyusb_cdcacm_write_queue(CDC_USB_DEV, &zero_byte, 1);

        // TODO: timeout size
        ESP_ERROR_CHECK_WITHOUT_ABORT(tinyusb_cdcacm_write_flush(CDC_USB_DEV, 1000));
    }
}

void usb_cdc_rx_callback(int itf, cdcacm_event_t* event) {
    if(gdb_glue_can_receive()) {
        size_t max_len = gdb_glue_get_free_size();
        if(max_len > CONFIG_USB_CDC_RX_BUFSIZE) max_len = CONFIG_USB_CDC_RX_BUFSIZE;
        size_t rx_size = 0;
        esp_err_t err = tinyusb_cdcacm_read(itf, buffer_rx, max_len, &rx_size);

        if(err == ESP_OK) {
            if(rx_size > 0) {
                gdb_glue_receive(buffer_rx, rx_size);
            }
        } else {
            ESP_LOGE(TAG, "Read error");
        }
    }
}

void usb_cdc_line_state_changed_callback(int itf, cdcacm_event_t* event) {
    usb_cdc.dtr = event->line_state_changed_data.dtr;
    usb_cdc.rts = event->line_state_changed_data.rts;

    ESP_LOGI(TAG, "Line state changed! dtr:%d, rst:%d", usb_cdc.dtr, usb_cdc.rts);
}

void usb_cdc_line_coding_changed_callback(int itf, cdcacm_event_t* event) {
    uint32_t bit_rate = event->line_coding_changed_data.p_line_coding->bit_rate;
    uint8_t stop_bits = event->line_coding_changed_data.p_line_coding->stop_bits;
    uint8_t parity = event->line_coding_changed_data.p_line_coding->parity;
    uint8_t data_bits = event->line_coding_changed_data.p_line_coding->data_bits;

    ESP_LOGI(
        TAG,
        "Line coding changed! bit_rate:%d, stop_bits:%d, parity:%d, data_bits:%d",
        bit_rate,
        stop_bits,
        parity,
        data_bits);
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// It seems like a reliable way is to rely on tud_mount_cb on connect and tud_suspend_cb on disconnect
void tud_mount_cb(void) {
    ESP_LOGI(TAG, "Mount");

    if(!usb_cdc.connected) {
        led_set_blue(255);
        delay(10);
        led_set_blue(0);
    }

    usb_cdc.connected = true;
}

void tud_umount_cb(void) {
    ESP_LOGI(TAG, "Unmount");

    if(usb_cdc.connected) {
        led_set_blue(255);
        delay(10);
        led_set_blue(0);
    }

    usb_cdc.connected = false;
}

void tud_resume_cb(void) {
    ESP_LOGI(TAG, "Resume");

    if(usb_cdc.connected) {
        led_set_blue(255);
        delay(10);
        led_set_blue(0);
    }

    usb_cdc.connected = true;
}

void tud_suspend_cb(bool remote_wakeup_en) {
    ESP_LOGI(TAG, "Suspend");

    if(usb_cdc.connected) {
        led_set_blue(255);
        delay(10);
        led_set_blue(0);
    }

    usb_cdc.connected = false;
}

void usb_cdc_init(void) {
    usb_cdc.connected = false;

    ESP_LOGI(TAG, "init");

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

    tinyusb_config_t tusb_cfg = {
        .descriptor = NULL, //Uses default descriptor specified in Menuconfig
        .string_descriptor = NULL, //Uses default string specified in Menuconfig
        .external_phy = false,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t amc_cfg = {
        .usb_dev = CDC_USB_DEV,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &usb_cdc_rx_callback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = &usb_cdc_line_state_changed_callback,
        .callback_line_coding_changed = &usb_cdc_line_coding_changed_callback};

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&amc_cfg));

    ESP_LOGI(TAG, "init done");
}
