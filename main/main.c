#include <stdint.h>
#include <esp_log.h>
#include "usb-cdc.h"
#include "nvs.h"
#include "wifi.h"

static const char* TAG = "main";

void app_main(void) {
    ESP_LOGI(TAG, "start");

    usb_cdc_init();
    nvs_init();
    wifi_init();

    ESP_LOGI(TAG, "end");
}
