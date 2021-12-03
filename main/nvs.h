/**
 * @file nvs.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-03
 * 
 * NVS management
 */
#pragma once
#include <m-string.h>
#include <esp_err.h>

void nvs_init(void);
void nvs_erase(void);
esp_err_t nvs_save_string(const char* key, const mstring_t* value);
esp_err_t nvs_load_string(const char* key, mstring_t* value);