/**
 * @file network.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-11-21
 * 
 * Network API
 */
#pragma once
#include <stdbool.h>

#define ESP_WIFI_MODE_STA "STA"
#define ESP_WIFI_MODE_AP "AP"

typedef enum {
    WIFIModeSTA,
    WIFIModeAP,
} WIFIMode;

/**
 * Init network module
 * @return WIFIMode
 */
WIFIMode network_init(void);

/**
 * Returns current WIFI mode
 * @return WIFIMode 
 */
WIFIMode network_get_mode(void);