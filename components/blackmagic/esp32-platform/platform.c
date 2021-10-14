#include <stdint.h>
// #include <hal\gpio_types.h>

uint32_t swd_delay_cnt = 0;

void platform_swdio_mode_float(void) {
    // gpio_set_direction(SWDIO_PIN, GPIO_MODE_INPUT);
    // gpio_set_pull_mode(SWDIO_PIN, GPIO_FLOATING);
}

void platform_swdio_mode_drive(void) {
    // gpio_set_direction(SWDIO_PIN, GPIO_MODE_OUTPUT);
}

void platform_gpio_set_level(int32_t gpio_num, uint32_t value) {
    /* TODO gpio_set_level(pin, value);*/
    /* ignore that -> sdk_os_delay_us(2);	*/
}

void platform_gpio_set(int32_t gpio_num) {
    // platform_gpio_set_level(gpio_num, 1);
}

void platform_gpio_clear(int32_t gpio_num) {
    // platform_gpio_set_level(gpio_num, 0);
}

int platform_gpio_get_level(int32_t gpio_num) {
    return 0;
}