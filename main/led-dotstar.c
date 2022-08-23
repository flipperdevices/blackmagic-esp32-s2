#include "led-dotstar.h"
#include <esp_log.h>
#include <esp_err.h>
#include "driver/gpio.h"

#define LED_COUNT 1
#define LED_CLK 12
#define LED_DAT 2
#define LED_PWR 13

#define TAG "led-dotstar"

static uint8_t _led_red = 0;
static uint8_t _led_green = 0;
static uint8_t _led_blue = 0;

void led_init() {
    ESP_LOGI(TAG, "init");

    init_led(LED_DAT, LED_CLK, LED_COUNT, DOTSTAR_BGR);
    gpio_set_level((gpio_num_t) 13, 1);

    ESP_LOGI(TAG, "init done");
}

void led_set(uint8_t red, uint8_t green, uint8_t blue) {
    for (int i = 0; i < LED_COUNT; i++) {
        setPixelColor(i, red, green, blue);
    }
    _led_red = red;
    _led_green = green;
    _led_blue = blue;
    printLED();
}

void led_set_red(uint8_t value) {
    _led_red = value;
    led_set(_led_red, _led_green, _led_blue);
    printLED();
}

void led_set_green(uint8_t value) {
    _led_green = value;
    led_set(_led_red, _led_green, _led_blue);
    printLED();
}

void led_set_blue(uint8_t value) {
    _led_blue = value;
    led_set(_led_red, _led_green, _led_blue);
    printLED();
}
