#pragma once

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022, Alex Taradov <alex@taradov.com>. All rights reserved.

/*- Includes ----------------------------------------------------------------*/
#include <stdint.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>
#include <hal/gpio_ll.h>
#include <esp_rom_gpio.h>
#include <esp_attr.h>

/*- Definitions -------------------------------------------------------------*/
// #define DAP_CONFIG_ENABLE_JTAG

#define DAP_CONFIG_DEFAULT_PORT DAP_PORT_SWD
#define DAP_CONFIG_DEFAULT_CLOCK 8000000 // Hz

#define DAP_CONFIG_PACKET_SIZE 64
#define DAP_CONFIG_PACKET_COUNT 1

#define DAP_CONFIG_JTAG_DEV_COUNT 8

// DAP_CONFIG_PRODUCT_STR must contain "CMSIS-DAP" to be compatible with the standard
#define DAP_CONFIG_VENDOR_STR "Flipper Devices"
#define DAP_CONFIG_PRODUCT_STR "ESP32S2 CMSIS-DAP Adapter"
#define DAP_CONFIG_SER_NUM_STR dap_serial_number
#define DAP_CONFIG_CMSIS_DAP_VER_STR "2.0.0"

// Attribute to use for performance-critical functions
#define DAP_CONFIG_PERFORMANCE_ATTR IRAM_ATTR

// A value at which dap_clock_test() produces 1 kHz output on the SWCLK pin
#define DAP_CONFIG_DELAY_CONSTANT 24000

// A threshold for switching to fast clock (no added delays)
// This is the frequency produced by dap_clock_test(1) on the SWCLK pin
#define DAP_CONFIG_FAST_CLOCK 8000000 // Hz

#define ESP_SWCLK_PIN (1)
#define ESP_SWDIO_PIN (2)

/*- Prototypes --------------------------------------------------------------*/
void dap_callback_connect(void);
void dap_callback_disconnect(void);
extern char dap_serial_number[32];
/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_write(int value) {
    if(value) {
        GPIO.out_w1ts = (1 << ESP_SWCLK_PIN);
    } else {
        GPIO.out_w1tc = (1 << ESP_SWCLK_PIN);
    }
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_write(int value) {
    if(value) {
        GPIO.out_w1ts = (1 << ESP_SWDIO_PIN);
    } else {
        GPIO.out_w1tc = (1 << ESP_SWDIO_PIN);
    }
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDI_write(int value) {
    // Do nothing
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDO_write(int value) {
    // Do nothing
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nTRST_write(int value) {
    // Do nothing
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nRESET_write(int value) {
    // Do nothing
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWCLK_TCK_read(void) {
    int level = (GPIO.in >> ESP_SWCLK_PIN) & 0x1;
    return level;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWDIO_TMS_read(void) {
    int level = (GPIO.in >> ESP_SWDIO_PIN) & 0x1;
    return level;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDO_read(void) {
    // Do nothing
    return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDI_read(void) {
    // Do nothing
    return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nTRST_read(void) {
    // Do nothing
    return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nRESET_read(void) {
    // Do nothing
    return 0;
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_set(void) {
    GPIO.out_w1ts = (1 << ESP_SWCLK_PIN);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_clr(void) {
    GPIO.out_w1tc = (1 << ESP_SWCLK_PIN);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_in(void) {
    gpio_ll_output_disable(&GPIO, ESP_SWDIO_PIN);
    gpio_ll_input_enable(&GPIO, ESP_SWDIO_PIN);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_out(void) {
    GPIO.enable_w1ts = (0x1 << ESP_SWDIO_PIN);
    esp_rom_gpio_connect_out_signal(ESP_SWDIO_PIN, SIG_GPIO_OUT_IDX, false, false);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SETUP(void) {
    // since the blackmagic probe is not have connect and disconnect callbacks
    // we can't enable the gpio

    // gpio_ll_output_disable(&GPIO, ESP_SWDIO_PIN);
    // gpio_ll_input_enable(&GPIO, ESP_SWDIO_PIN);
    // gpio_ll_output_disable(&GPIO, ESP_SWCLK_PIN);
    // gpio_ll_input_enable(&GPIO, ESP_SWCLK_PIN);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_DISCONNECT(void) {
    // since the blackmagic probe is not have connect and disconnect callbacks
    // we can't disable the gpio

    // gpio_ll_output_disable(&GPIO, ESP_SWDIO_PIN);
    // gpio_ll_input_enable(&GPIO, ESP_SWDIO_PIN);
    // gpio_ll_output_disable(&GPIO, ESP_SWCLK_PIN);
    // gpio_ll_input_enable(&GPIO, ESP_SWCLK_PIN);

    dap_callback_disconnect();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_SWD(void) {
    GPIO.enable_w1ts = (0x1 << ESP_SWDIO_PIN);
    esp_rom_gpio_connect_out_signal(ESP_SWDIO_PIN, SIG_GPIO_OUT_IDX, false, false);

    GPIO.enable_w1ts = (0x1 << ESP_SWCLK_PIN);
    esp_rom_gpio_connect_out_signal(ESP_SWCLK_PIN, SIG_GPIO_OUT_IDX, false, false);

    dap_callback_connect();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_JTAG(void) {
    // Do nothing
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_LED(int index, int state) {
    // Do nothing
}

//-----------------------------------------------------------------------------
__attribute__((always_inline)) static inline void DAP_CONFIG_DELAY(uint32_t cycles) {
    register int32_t cnt;
    for(cnt = cycles; --cnt > 0;)
        ;
}
