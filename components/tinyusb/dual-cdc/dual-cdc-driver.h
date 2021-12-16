/**
 * @file dual-cdc-driver.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-16
 * 
 * Dual CDC driver
 */

#pragma once

#include <stdint.h>
#include <esp_err.h>
#include <tusb_config.h>

#define CDC_IF_COUNT CFG_TUD_CDC

/**
 * 
 * @return esp_err_t 
 */
esp_err_t dual_cdc_driver_install(void);
