#ifndef DOTSTAR_H
#define DOTSTAR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "esp_log.h"

enum COLOR_MODE{
    DOTSTAR_RGB=0,
    DOTSTAR_GRB,
    DOTSTAR_BGR,
};

void init_led(uint8_t data_pin, uint8_t clock_pin, uint8_t num_of_leds, uint8_t color_mode);
int setPixel24bitColor(uint8_t led_index, uint32_t color);
int setPixelColor(uint8_t led_index,uint8_t r,uint8_t g,uint8_t b);
int printLED();

#endif
