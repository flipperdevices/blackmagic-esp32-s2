#include <stdint.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <tinyusb.h>
#include <tusb_cdc_acm.h>
#include <sdkconfig.h>

static const char* TAG = "usb-cdc";
static uint8_t buf[CONFIG_USB_CDC_RX_BUFSIZE + 1];

void usb_cdc_rx_callback(int itf, cdcacm_event_t* event) {
    size_t rx_size = 0;

    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_USB_CDC_RX_BUFSIZE, &rx_size);
    if(ret == ESP_OK) {
        buf[rx_size] = '\0';
        ESP_LOGI(TAG, "Got data (%d bytes): %s", rx_size, buf);
    } else {
        ESP_LOGE(TAG, "Read error");
    }

    // echo
    tinyusb_cdcacm_write_queue(itf, buf, rx_size);
    tinyusb_cdcacm_write_flush(itf, 0);
}

void usb_cdc_line_state_changed_callback(int itf, cdcacm_event_t* event) {
    int dtr = event->line_state_changed_data.dtr;
    int rst = event->line_state_changed_data.rts;

    ESP_LOGI(TAG, "Line state changed! dtr:%d, rst:%d", dtr, rst);
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

void usb_cdc_init(void) {
    ESP_LOGI(TAG, "USB initialization");
    tinyusb_config_t tusb_cfg = {};
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

    ESP_LOGI(TAG, "USB initialization DONE");
}
