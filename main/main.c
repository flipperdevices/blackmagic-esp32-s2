#include <stdint.h>
#include <esp_log.h>
#include "usb-cdc.h"
#include "nvs.h"
#include "wifi.h"
#include "gdb_main.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rom/ets_sys.h>

static const char* TAG = "main";

void gdb_application_thread(void* pvParameters) {
    ESP_LOGI("gdb", "start");
    while(1) {
        gdb_main();
    }
    ESP_LOGI("gdb", "end");
}

#include <platform.h>
#include <driver/gpio.h>

void pins_init() {
    gpio_config_t io_conf;
    // disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = ((1 << SWCLK_PIN) | (1 << SWDIO_PIN));
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
}

void app_main(void) {
    ESP_LOGI(TAG, "start");

    usb_cdc_init();
    // nvs_init();
    // wifi_init();

    pins_init();

    xTaskCreate(&gdb_application_thread, "gdb_thread", 16 * 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "end");
}
