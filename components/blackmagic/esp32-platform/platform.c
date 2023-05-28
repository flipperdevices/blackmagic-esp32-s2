#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "general.h"
#include <esp_log.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>

#include <hal/gpio_ll.h>
#include <esp_rom_gpio.h>
#include "esp_timer.h"

uint32_t swd_delay_cnt = 0;
// static const char* TAG = "gdb-platform";

void __attribute__((always_inline)) platform_swdio_mode_float(void) {
    // gpio_set_direction(SWDIO_PIN, GPIO_MODE_INPUT);
    // gpio_set_pull_mode(SWDIO_PIN, GPIO_FLOATING);

    // Faster variant
    gpio_ll_output_disable(&GPIO, SWDIO_PIN);
    gpio_ll_input_enable(&GPIO, SWDIO_PIN);
}

void __attribute__((always_inline)) platform_swdio_mode_drive(void) {
    // gpio_set_direction(SWDIO_PIN, GPIO_MODE_OUTPUT);

    // Faster variant
    // Supports only gpio less than 32
    GPIO.enable_w1ts = (0x1 << SWDIO_PIN);
    esp_rom_gpio_connect_out_signal(SWDIO_PIN, SIG_GPIO_OUT_IDX, false, false);
}

void __attribute__((always_inline)) platform_gpio_set_level(int32_t gpio_num, uint32_t value) {
    // gpio_set_level(gpio_num, value);

    // Faster variant
    // Supports only gpio less than 32
    if(value) {
        GPIO.out_w1ts = (1 << gpio_num);
    } else {
        GPIO.out_w1tc = (1 << gpio_num);
    }
}

void __attribute__((always_inline)) platform_gpio_set(int32_t gpio_num) {
    // platform_gpio_set_level(gpio_num, 1);

    // Faster variant
    // Supports only gpio less than 32
    GPIO.out_w1ts = (1 << gpio_num);
}

void __attribute__((always_inline)) platform_gpio_clear(int32_t gpio_num) {
    // platform_gpio_set_level(gpio_num, 0);

    // faster variant
    // supports only gpio less than 32
    GPIO.out_w1tc = (1 << gpio_num);
}

int __attribute__((always_inline)) platform_gpio_get_level(int32_t gpio_num) {
    // int level = gpio_get_level(gpio_num);

    // Faster variant
    // Supports only gpio less than 32
    int level = (GPIO.in >> gpio_num) & 0x1;
    return level;
}

// init platform
void platform_init() {
}

// set reset target pin level
void platform_srst_set_val(bool assert) {
    (void)assert;
}

// get reset target pin level
bool platform_srst_get_val(void) {
    return false;
}

// target voltage
const char* platform_target_voltage(void) {
    return NULL;
}

// platform time counter
uint32_t platform_time_ms(void) {
    int64_t time_milli = esp_timer_get_time() / 1000;
    return ((uint32_t)time_milli);
}

// delay ms
void platform_delay(uint32_t ms) {
    vTaskDelay((ms) / portTICK_PERIOD_MS);
}

// hardware version
int platform_hwversion(void) {
    return 0;
}

// set timeout
void platform_timeout_set(platform_timeout_s* t, uint32_t ms) {
    t->time = platform_time_ms() + ms;
}

// check timeout
bool platform_timeout_is_expired(const platform_timeout_s* t) {
    return platform_time_ms() > t->time;
}

// set interface freq
void platform_max_frequency_set(uint32_t freq) {
}

// get interface freq
uint32_t platform_max_frequency_get(void) {
    return 0;
}

void platform_nrst_set_val(bool assert) {
    (void)assert;
}

bool platform_nrst_get_val() {
    return false;
}

void platform_target_clk_output_enable(bool enable) {
    (void)enable;
}
