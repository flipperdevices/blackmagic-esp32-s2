#pragma once
#include <timing.h>

extern uint32_t swd_delay_cnt;

void platform_swdio_mode_float(void);
void platform_swdio_mode_drive(void);
void platform_gpio_set_level(int32_t gpio_num, uint32_t value);
void platform_gpio_set(int32_t gpio_num);
void platform_gpio_clear(int32_t gpio_num);
int platform_gpio_get_level(int32_t gpio_num);

#define PLATFORM_IDENT "ESP32-S2"

#define NO_USB_PLEASE

#define SET_RUN_STATE(state)
#define SET_IDLE_STATE(state)
#define SET_ERROR_STATE(state)

#define TMS_SET_MODE() \
    do {               \
    } while(0)

#if 1

#define TMS_PIN (-1)
#define TDI_PIN (-1)
#define TDO_PIN (-1)
#define TCK_PIN (-1)

#undef PLATFORM_HAS_TRACESWO
#define TRACESWO_PIN 18
#endif

// ON ESP32 we dont have the PORTS, this is dummy value until code is corrected
#define SWCLK_PORT (0)
#define SWCLK_PIN (1)
#define SWDIO_PIN (2)

#if SWDIO_PIN >= 32 || SWCLK_PIN >= 32
#error To support pins greater than 31, change the platform_gpio functions
#endif

#define gpio_set_val(port, pin, value)       \
    do {                                     \
        platform_gpio_set_level(pin, value); \
    } while(0);

#define gpio_set(port, pin) platform_gpio_set(pin)
#define gpio_clear(port, pin) platform_gpio_clear(pin)
#define gpio_get(port, pin) platform_gpio_get_level(pin)

#define SWDIO_MODE_FLOAT()           \
    do {                             \
        platform_swdio_mode_float(); \
    } while(0)

#define SWDIO_MODE_DRIVE()           \
    do {                             \
        platform_swdio_mode_drive(); \
    } while(0)
