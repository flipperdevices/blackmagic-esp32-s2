#include "led.h"
#include <driver/ledc.h>
#include <esp_err.h>

#define LED_PIN_RED (4)
#define LED_PIN_GREEN (5)
#define LED_PIN_BLUE (6)

#define LEDC_MODE LEDC_LOW_SPEED_MODE

typedef enum {
    LedChannelRed,
    LedChannelGreen,
    LedChannelBlue,
} ledc_channel;

void led_init() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000, // Set output frequency at 5 kHz
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel_red = {
        .speed_mode = LEDC_MODE,
        .channel = LedChannelRed,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LED_PIN_RED,
        .duty = 256, // Set duty to 100%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_red));

    ledc_channel_config_t ledc_channel_green = {
        .speed_mode = LEDC_MODE,
        .channel = LedChannelGreen,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LED_PIN_GREEN,
        .duty = 256, // Set duty to 100%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_green));

    ledc_channel_config_t ledc_channel_blue = {
        .speed_mode = LEDC_MODE,
        .channel = LedChannelBlue,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LED_PIN_BLUE,
        .duty = 256, // Set duty to 100%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_blue));
}

void led_set(uint8_t red, uint8_t green, uint8_t blue) {
    led_set_red(red);
    led_set_green(green);
    led_set_blue(blue);
}

void led_set_red(uint8_t value) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LedChannelRed, 256U - value));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LedChannelRed));
}

void led_set_green(uint8_t value) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LedChannelGreen, 256U - value));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LedChannelGreen));
}

void led_set_blue(uint8_t value) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LedChannelBlue, 256U - value));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LedChannelBlue));
}
