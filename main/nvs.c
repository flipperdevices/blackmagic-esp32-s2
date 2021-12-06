#include <nvs_flash.h>
#include <esp_log.h>
#include "nvs.h"

#define TAG "nvs"
#define NVS_STORE "nvs_storage"

void nvs_init(void) {
    ESP_LOGI(TAG, "init " NVS_DEFAULT_PART_NAME);
    esp_err_t ret = nvs_flash_init_partition(NVS_DEFAULT_PART_NAME);
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "erasing " NVS_DEFAULT_PART_NAME);
        ESP_ERROR_CHECK(nvs_flash_erase_partition(NVS_DEFAULT_PART_NAME));
        ret = nvs_flash_init_partition(NVS_DEFAULT_PART_NAME);
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "init " NVS_STORE);
    ret = nvs_flash_init_partition(NVS_STORE);
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "erasing " NVS_STORE);
        ESP_ERROR_CHECK(nvs_flash_erase_partition(NVS_STORE));
        ret = nvs_flash_init_partition(NVS_STORE);
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "init done");
}

void nvs_erase(void) {
    ESP_LOGI(TAG, "erasing " NVS_DEFAULT_PART_NAME);
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(TAG, "erasing " NVS_STORE);
    ESP_ERROR_CHECK(nvs_flash_erase_partition(NVS_STORE));
    ESP_ERROR_CHECK(nvs_flash_init_partition(NVS_STORE));

    ESP_LOGI(TAG, "erasing done");
}

esp_err_t nvs_save_string(const char* key, const mstring_t* value) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    do {
        err = nvs_open_from_partition(NVS_STORE, "config", NVS_READWRITE, &nvs_handle);
        if(err != ESP_OK) break;

        err = nvs_set_str(nvs_handle, key, mstring_get_cstr(value));
        if(err != ESP_OK) break;

        err = nvs_commit(nvs_handle);
        if(err != ESP_OK) break;

        nvs_close(nvs_handle);
        err = ESP_OK;
    } while(0);

    return err;
}

esp_err_t nvs_load_string(const char* key, mstring_t* value) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    char* buffer = NULL;

    do {
        err = nvs_open_from_partition(NVS_STORE, "config", NVS_READONLY, &nvs_handle);
        if(err != ESP_OK) break;

        size_t required_size = 0;
        err = nvs_get_str(nvs_handle, key, NULL, &required_size);
        if(err != ESP_OK) break;

        buffer = malloc(required_size + sizeof(uint32_t));
        err = nvs_get_str(nvs_handle, key, buffer, &required_size);
        if(err != ESP_OK) break;

        mstring_set(value, buffer);
        nvs_close(nvs_handle);
        err = ESP_OK;
    } while(0);

    if(buffer != NULL) free(buffer);

    return err;
}