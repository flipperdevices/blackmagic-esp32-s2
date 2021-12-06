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
#include "nvs-config.h"

/**
 * Init network module
 * @return WiFiMode
 */
WiFiMode network_init(void);

/**
 * Returns current WIFI mode
 * @return WiFiMode 
 */
WiFiMode network_get_mode(void);

/**
 * Get current IP address
 * @return uint32_t 
 */
uint32_t network_get_ip(void);