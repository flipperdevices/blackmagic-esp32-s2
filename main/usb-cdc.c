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
#include "usb-cdc.h"
#include "led.h"
#include "delay.h"
#include <driver/gpio.h>

#define USB_DN_PIN (19)
#define USB_DP_PIN (20)

static const char* TAG = "usb-cdc";
static uint8_t buffer_rx[CONFIG_USB_CDC_RX_BUFSIZE + 1];

typedef struct {
    volatile bool connected;
    volatile bool dtr;
    volatile bool rts;

    StreamBufferHandle_t rx_stream;
    volatile bool rx_stream_full;
} FuriHalVcp;

static FuriHalVcp furi_hal_vcp;

size_t usb_cdc_rx(uint8_t* buffer, size_t size) {
    return usb_cdc_rx_with_timeout(buffer, size, portMAX_DELAY);
}

size_t usb_cdc_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout) {
    size_t received = xStreamBufferReceive(furi_hal_vcp.rx_stream, buffer, size, timeout);

    if(furi_hal_vcp.rx_stream_full &&
       xStreamBufferSpacesAvailable(furi_hal_vcp.rx_stream) >= CONFIG_USB_CDC_RX_BUFSIZE) {
        furi_hal_vcp.rx_stream_full = false;
        ESP_LOGW(TAG, "Stream freed");
    }

    return received;
}

void usb_cdc_tx_char(uint8_t c, bool flush) {
    // TinyUSB implements buffering, no buffering is required here
    tinyusb_cdcacm_write_queue(TINYUSB_USBDEV_0, &c, 1);

    if(flush) {
        // SOME GDB MAGIC
        // We need to send an empty packet for some hosts to accept this as a complete transfer.
        uint8_t zero_byte = 0;
        tinyusb_cdcacm_write_queue(TINYUSB_USBDEV_0, &zero_byte, 1);

        // TODO: timeout size
        ESP_ERROR_CHECK_WITHOUT_ABORT(tinyusb_cdcacm_write_flush(TINYUSB_USBDEV_0, 1000));
    }
}

void usb_cdc_rx_callback(int itf, cdcacm_event_t* event) {
    uint16_t max_len = xStreamBufferSpacesAvailable(furi_hal_vcp.rx_stream);

    if(max_len > 0) {
        if(max_len > CONFIG_USB_CDC_RX_BUFSIZE) max_len = CONFIG_USB_CDC_RX_BUFSIZE;
        size_t rx_size = 0;
        esp_err_t err = tinyusb_cdcacm_read(itf, buffer_rx, max_len, &rx_size);

        if(err == ESP_OK) {
            if(rx_size > 0) {
                size_t ret =
                    xStreamBufferSend(furi_hal_vcp.rx_stream, buffer_rx, rx_size, portMAX_DELAY);
                ESP_ERROR_CHECK(ret != rx_size);
                // buffer_rx[rx_size] = '\0';
                // printf("%s", buffer_rx);
            }
        } else {
            ESP_LOGE(TAG, "Read error");
        }

    } else {
        furi_hal_vcp.rx_stream_full = true;
        ESP_LOGE(TAG, "Stream is full");
    };
}

void usb_cdc_line_state_changed_callback(int itf, cdcacm_event_t* event) {
    furi_hal_vcp.dtr = event->line_state_changed_data.dtr;
    furi_hal_vcp.rts = event->line_state_changed_data.rts;

    ESP_LOGI(TAG, "Line state changed! dtr:%d, rst:%d", furi_hal_vcp.dtr, furi_hal_vcp.rts);
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

    if(!furi_hal_vcp.connected) {
        led_set_blue(255);
        delay(10);
        led_set_blue(0);
    }

    furi_hal_vcp.connected = true;
}

void tud_umount_cb(void) {
    ESP_LOGI(TAG, "Unmount");

    if(furi_hal_vcp.connected) {
        led_set_blue(255);
        delay(10);
        led_set_blue(0);
    }

    furi_hal_vcp.connected = false;
}

void tud_resume_cb(void) {
    ESP_LOGI(TAG, "Resume");

    if(furi_hal_vcp.connected) {
        led_set_blue(255);
        delay(10);
        led_set_blue(0);
    }

    furi_hal_vcp.connected = true;
}

void tud_suspend_cb(bool remote_wakeup_en) {
    ESP_LOGI(TAG, "Suspend");

    if(furi_hal_vcp.connected) {
        led_set_blue(255);
        delay(10);
        led_set_blue(0);
    }

    furi_hal_vcp.connected = false;
}

void usb_cdc_init(void) {
    furi_hal_vcp.connected = false;
    furi_hal_vcp.rx_stream = xStreamBufferCreate((CONFIG_USB_CDC_RX_BUFSIZE * 16), 1);
    furi_hal_vcp.rx_stream_full = false;

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
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &usb_cdc_rx_callback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = &usb_cdc_line_state_changed_callback,
        .callback_line_coding_changed = &usb_cdc_line_coding_changed_callback};

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&amc_cfg));

    ESP_LOGI(TAG, "init done");
}
