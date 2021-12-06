/**
 * @file nvs-config.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-03
 * 
 * Configuration helper
 */
#pragma once

#include <m-string.h>
#include <esp_err.h>

#define CFG_WIFI_MODE_AP "AP"
#define CFG_WIFI_MODE_STA "STA"

typedef enum {
    WiFiModeAP, // host of a WiFi network
    WiFiModeSTA, // connected to existing WiFi AP
} WiFiMode;

esp_err_t nvs_config_set_wifi_mode(WiFiMode value);
esp_err_t nvs_config_set_ap_ssid(const mstring_t* ssid);
esp_err_t nvs_config_set_ap_pass(const mstring_t* pass);
esp_err_t nvs_config_set_sta_ssid(const mstring_t* ssid);
esp_err_t nvs_config_set_sta_pass(const mstring_t* pass);

esp_err_t nvs_config_get_wifi_mode(WiFiMode* value);
esp_err_t nvs_config_get_ap_ssid(mstring_t* ssid);
esp_err_t nvs_config_get_ap_pass(mstring_t* pass);
esp_err_t nvs_config_get_sta_ssid(mstring_t* ssid);
esp_err_t nvs_config_get_sta_pass(mstring_t* pass);