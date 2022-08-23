#include <stdint.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rom/ets_sys.h>

#include "usb-uart.h"
#include "nvs.h"
#include "gdb_main.h"
// #include "led.h"
#include "led-dotstar.h"
#include "cli-uart.h"
#include "i2c.h"
#include "network.h"
#include "network-http.h"
#include "network-uart.h"
#include "network-gdb.h"
#include "factory-reset-service.h"

#include <gdb-glue.h>
#include <soft-uart-log.h>

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
    // Software UART logging at pin 7, 57600 baud
    soft_uart_log_init(7, 57600);

    ESP_LOGI(TAG, "start");

    factory_reset_service_init();

    gdb_glue_init();

    led_init();
    led_set_blue(255);

    nvs_init();
    network_init();
    network_http_server_init();
    // network_uart_server_init();
    network_gdb_server_init();

    // usb_cdc_init();
    cli_uart_init();

    // TODO uart and i2c share the same pins, need switching mechanics
    // i2c_init();
    // i2c_scan();

    pins_init();
    xTaskCreate(&gdb_application_thread, "gdb_thread", 4096, NULL, 5, NULL);
    led_set_blue(0);
    ESP_LOGI(TAG, "end");
}