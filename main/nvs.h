#pragma once
#include <m-string.h>
#include <esp_err.h>

void nvs_init(void);
esp_err_t nvs_save_string(const char* key, const mstring_t* value);
esp_err_t nvs_load_string(const char* key, mstring_t* value);