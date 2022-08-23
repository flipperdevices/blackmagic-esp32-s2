
#include <esp_http_server.h>
#include <svelte-portal.h>
#include <esp_log.h>
#include <cJSON.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include "network.h"
#include "nvs.h"
#include "nvs-config.h"
#include "led-dotstar.h"
#include "helpers.h"

#define TAG "network-http"
#define JSON_ERROR(error_text) "{\"error\": \"" error_text "\"}"
#define JSON_RESULT(result_text) "{\"result\": \"" result_text "\"}"

#define WIFI_SCAN_SIZE 20

static const char* get_auth_mode(int authmode) {
    switch(authmode) {
    case WIFI_AUTH_OPEN:
        return "OPEN";
    case WIFI_AUTH_WEP:
        return "WEP";
    case WIFI_AUTH_WPA_PSK:
        return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
        return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
        return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return "WPA2_ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK:
        return "WPA3_PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return "WPA2_WPA3_PSK";
    default:
        return "UNKNOWN";
    }
}

static const char* get_pairwise_cipher(int pairwise_cipher) {
    switch(pairwise_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        return "NONE";
    case WIFI_CIPHER_TYPE_WEP40:
        return "WEP40";
    case WIFI_CIPHER_TYPE_WEP104:
        return "WEP104";
    case WIFI_CIPHER_TYPE_TKIP:
        return "TKIP";
    case WIFI_CIPHER_TYPE_CCMP:
        return "CCMP";
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        return "TKIP_CCMP";
    default:
        return "UNKNOWN";
    }
}

static const char* get_group_cipher(int group_cipher) {
    switch(group_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        return "NONE";
    case WIFI_CIPHER_TYPE_WEP40:
        return "WEP40";
    case WIFI_CIPHER_TYPE_WEP104:
        return "WEP104";
    case WIFI_CIPHER_TYPE_TKIP:
        return "TKIP";
    case WIFI_CIPHER_TYPE_CCMP:
        return "CCMP";
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        return "TKIP_CCMP";
    default:
        return "UNKNOWN";
    }
}

typedef struct {
    const char* uri;
    const char* type;
    const uint8_t* addr_start;
    const uint8_t* addr_end;
} HttpResource;

const HttpResource http_resources[] = {
    {
        .uri = "/",
        .type = "text/html",
        .addr_start = index_html_start,
        .addr_end = index_html_end,
    },
    {
        .uri = "/index.html",
        .type = "text/html",
        .addr_start = index_html_start,
        .addr_end = index_html_end,
    },
    {
        .uri = "/build/bundle.css",
        .type = "text/css",
        .addr_start = build_bundle_css_start,
        .addr_end = build_bundle_css_end,
    },
    {
        .uri = "/build/bundle.js",
        .type = "application/javascript",
        .addr_start = build_bundle_js_start,
        .addr_end = build_bundle_js_end,
    },
    {
        .uri = "/assets/ega8.otf",
        .type = "application/x-font-opentype",
        .addr_start = assets_ega8_otf_start,
        .addr_end = assets_ega8_otf_end,
    },
    {
        .uri = "/assets/favicon.ico",
        .type = "image/x-icon",
        .addr_start = assets_favicon_ico_start,
        .addr_end = assets_favicon_ico_end,
    },
    {
        .uri = "/favicon.ico",
        .type = "image/x-icon",
        .addr_start = assets_favicon_ico_start,
        .addr_end = assets_favicon_ico_end,
    },
};

static esp_err_t http_common_get_handler(httpd_req_t* req) {
    const uint8_t* file_start = NULL;
    const uint8_t* file_end = NULL;

    for(size_t i = 0; i < COUNT_OF(http_resources); i++) {
        if(strcmp(http_resources[i].uri, req->uri) == 0) {
            file_start = http_resources[i].addr_start;
            file_end = http_resources[i].addr_end;
            httpd_resp_set_type(req, http_resources[i].type);
            break;
        }
    }

    if(file_start == NULL) {
        ESP_LOGE(TAG, "file not exist: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "file not exist");
        return ESP_FAIL;
    }

    esp_err_t err =
        httpd_resp_send_chunk(req, (const char*)file_start, (ssize_t)(file_end - file_start));
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "file sending failed, %d", err);
        // Abort
        httpd_resp_sendstr_chunk(req, NULL);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "file sending failed");
        return ESP_FAIL;
    } else {
        // HTTP response completion
        httpd_resp_send_chunk(req, NULL, 0);
    }

    ESP_LOGI(TAG, "file sending complete");
    return ESP_OK;
}

static esp_err_t system_ping_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, JSON_RESULT("OK"));
    return ESP_OK;
}

static esp_err_t system_tasks_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");

    uint32_t task_count = uxTaskGetNumberOfTasks();
    TaskStatus_t* tasks = malloc(task_count * sizeof(TaskStatus_t));
    task_count = uxTaskGetSystemState(tasks, task_count, NULL);

    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "count", task_count);
    cJSON* array = cJSON_AddArrayToObject(root, "list");

    for(uint32_t i = 0; i < task_count; i++) {
        cJSON* object = cJSON_CreateObject();
        cJSON_AddNumberToObject(object, "handle", (uint32_t)tasks[i].xHandle);
        cJSON_AddStringToObject(object, "name", tasks[i].pcTaskName);
        cJSON_AddNumberToObject(object, "number", (uint32_t)tasks[i].xTaskNumber);

        switch(tasks[i].eCurrentState) {
        case eRunning:
            cJSON_AddStringToObject(object, "state", "Running");
            break;
        case eReady:
            cJSON_AddStringToObject(object, "state", "Ready");
            break;
        case eBlocked:
            cJSON_AddStringToObject(object, "state", "Blocked");
            break;
        case eSuspended:
            cJSON_AddStringToObject(object, "state", "Suspended");
            break;
        case eDeleted:
            cJSON_AddStringToObject(object, "state", "Deleted");
            break;
        case eInvalid:
            cJSON_AddStringToObject(object, "state", "Invalid");
            break;
        }
        cJSON_AddNumberToObject(object, "current_priority", (uint32_t)tasks[i].uxCurrentPriority);
        cJSON_AddNumberToObject(object, "base_priority", (uint32_t)tasks[i].uxBasePriority);
        cJSON_AddNumberToObject(object, "runtime", (uint32_t)tasks[i].ulRunTimeCounter);
        cJSON_AddNumberToObject(object, "stack_base", (uint32_t)tasks[i].pxStackBase);
        cJSON_AddNumberToObject(object, "watermark", (uint32_t)tasks[i].usStackHighWaterMark);

        cJSON_AddItemToArray(array, object);
    }

    const char* json_text = cJSON_Print(root);
    httpd_resp_sendstr(req, json_text);
    free((void*)json_text);
    cJSON_Delete(root);
    free((void*)tasks);

    return ESP_OK;
}

static esp_err_t system_reboot(httpd_req_t* req) {
    httpd_resp_sendstr(req, JSON_RESULT("OK"));
    esp_restart();
    return ESP_OK;
}

static esp_err_t system_info_get_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cJSON_AddStringToObject(root, "idf_version", IDF_VER);
    switch(chip_info.model) {
    case CHIP_ESP32:
        cJSON_AddStringToObject(root, "model", "ESP32");
        break;
    case CHIP_ESP32S2:
        cJSON_AddStringToObject(root, "model", "ESP32-S2");
        break;
    case CHIP_ESP32H2:
        cJSON_AddStringToObject(root, "model", "ESP32-H2");
        break;
    case CHIP_ESP32S3:
        cJSON_AddStringToObject(root, "model", "ESP32-S3");
        break;
    case CHIP_ESP32C3:
        cJSON_AddStringToObject(root, "model", "ESP32-C3");
        break;
    default:
        cJSON_AddStringToObject(root, "model", "UNKNOWN");
        break;
    }
    cJSON_AddNumberToObject(root, "revision", chip_info.revision);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);

    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    cJSON* heap = cJSON_AddObjectToObject(root, "heap");
    cJSON_AddNumberToObject(heap, "total_free_bytes", info.total_free_bytes);
    cJSON_AddNumberToObject(heap, "total_allocated_bytes", info.total_allocated_bytes);
    cJSON_AddNumberToObject(heap, "largest_free_block", info.largest_free_block);
    cJSON_AddNumberToObject(heap, "minimum_free_bytes", info.minimum_free_bytes);

    // ip addr
    cJSON_AddNumberToObject(root, "ip", network_get_ip());

    // mac
    uint8_t mac_addr[6] = {0};
    ESP_ERROR_CHECK(esp_read_mac(mac_addr, ESP_MAC_WIFI_STA));

    cJSON* mac = cJSON_AddArrayToObject(root, "mac");
    cJSON_AddItemToArray(mac, cJSON_CreateNumber(mac_addr[0]));
    cJSON_AddItemToArray(mac, cJSON_CreateNumber(mac_addr[1]));
    cJSON_AddItemToArray(mac, cJSON_CreateNumber(mac_addr[2]));
    cJSON_AddItemToArray(mac, cJSON_CreateNumber(mac_addr[3]));
    cJSON_AddItemToArray(mac, cJSON_CreateNumber(mac_addr[4]));
    cJSON_AddItemToArray(mac, cJSON_CreateNumber(mac_addr[5]));

    const char* json_text = cJSON_Print(root);
    httpd_resp_sendstr(req, json_text);
    free((void*)json_text);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t wifi_get_credentials_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();

    mstring_t* ap_ssid = mstring_alloc();
    mstring_t* ap_pass = mstring_alloc();
    mstring_t* sta_ssid = mstring_alloc();
    mstring_t* sta_pass = mstring_alloc();
    WiFiMode wifi_mode;

    nvs_config_get_wifi_mode(&wifi_mode);
    nvs_config_get_ap_ssid(ap_ssid);
    nvs_config_get_ap_pass(ap_pass);
    nvs_config_get_sta_ssid(sta_ssid);
    nvs_config_get_sta_pass(sta_pass);

    cJSON_AddStringToObject(root, "ap_ssid", mstring_get_cstr(ap_ssid));
    cJSON_AddStringToObject(root, "ap_pass", mstring_get_cstr(ap_pass));
    cJSON_AddStringToObject(root, "sta_ssid", mstring_get_cstr(sta_ssid));
    cJSON_AddStringToObject(root, "sta_pass", mstring_get_cstr(sta_pass));

    switch(wifi_mode) {
    case WiFiModeAP:
        cJSON_AddStringToObject(root, "wifi_mode", CFG_WIFI_MODE_AP);
        break;
    case WiFiModeSTA:
        cJSON_AddStringToObject(root, "wifi_mode", CFG_WIFI_MODE_STA);
        break;
    }

    const char* json_text = cJSON_Print(root);
    httpd_resp_sendstr(req, json_text);
    free((void*)json_text);
    cJSON_Delete(root);
    mstring_free(ap_ssid);
    mstring_free(ap_pass);
    mstring_free(sta_ssid);
    mstring_free(sta_pass);
    return ESP_OK;
}

static esp_err_t wifi_set_credentials_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    int total_length = req->content_len;
    int cur_len = 0;
    char* buffer = malloc(256);
    mstring_t* ap_ssid = mstring_alloc();
    mstring_t* ap_pass = mstring_alloc();
    mstring_t* sta_ssid = mstring_alloc();
    mstring_t* sta_pass = mstring_alloc();
    mstring_t* wifi_mode = mstring_alloc();
    const char* error_text = JSON_ERROR("unknown error");
    int received = 0;
    httpd_resp_set_type(req, "application/json");

    if(total_length >= 256) {
        error_text = JSON_ERROR("request too long");
        goto err_fail;
    }

    while(cur_len < total_length) {
        received = httpd_req_recv(req, buffer + cur_len, total_length);
        if(received <= 0) {
            error_text = JSON_ERROR("cannot receive request data");
            goto err_fail;
        }
        cur_len += received;
    }
    buffer[total_length] = '\0';

    cJSON* root = cJSON_Parse(buffer);
    if(cJSON_GetObjectItem(root, "ap_ssid") != NULL) {
        mstring_set(ap_ssid, cJSON_GetObjectItem(root, "ap_ssid")->valuestring);
    } else {
        cJSON_Delete(root);
        error_text = JSON_ERROR("request dont have [ap_ssid] field");
        goto err_fail;
    }

    if(cJSON_GetObjectItem(root, "ap_pass") != NULL) {
        mstring_set(ap_pass, cJSON_GetObjectItem(root, "ap_pass")->valuestring);
    } else {
        cJSON_Delete(root);
        error_text = JSON_ERROR("request dont have [ap_pass] field");
        goto err_fail;
    }

    if(cJSON_GetObjectItem(root, "sta_ssid") != NULL) {
        mstring_set(sta_ssid, cJSON_GetObjectItem(root, "sta_ssid")->valuestring);
    } else {
        cJSON_Delete(root);
        error_text = JSON_ERROR("request dont have [sta_ssid] field");
        goto err_fail;
    }

    if(cJSON_GetObjectItem(root, "sta_pass") != NULL) {
        mstring_set(sta_pass, cJSON_GetObjectItem(root, "sta_pass")->valuestring);
    } else {
        cJSON_Delete(root);
        error_text = JSON_ERROR("request dont have [sta_pass] field");
        goto err_fail;
    }

    if(cJSON_GetObjectItem(root, "wifi_mode") != NULL) {
        mstring_set(wifi_mode, cJSON_GetObjectItem(root, "wifi_mode")->valuestring);
    } else {
        cJSON_Delete(root);
        error_text = JSON_ERROR("request dont have [wifi_mode] field");
        goto err_fail;
    }
    cJSON_Delete(root);

    if(strcmp(mstring_get_cstr(wifi_mode), CFG_WIFI_MODE_AP) != 0 &&
       strcmp(mstring_get_cstr(wifi_mode), CFG_WIFI_MODE_STA) != 0) {
        error_text = JSON_ERROR("invalid value in [wifi_mode]");
        goto err_fail;
    }

    if(nvs_config_set_ap_ssid(ap_ssid) != ESP_OK) {
        error_text = JSON_ERROR("invalid value in [ap_ssid]");
        goto err_fail;
    }
    if(nvs_config_set_ap_pass(ap_pass) != ESP_OK) {
        error_text = JSON_ERROR("invalid value in [ap_pass]");
        goto err_fail;
    }
    if(nvs_config_set_sta_ssid(sta_ssid) != ESP_OK) {
        error_text = JSON_ERROR("invalid value in [sta_ssid]");
        goto err_fail;
    }
    if(nvs_config_set_sta_pass(sta_pass) != ESP_OK) {
        error_text = JSON_ERROR("invalid value in [sta_pass]");
        goto err_fail;
    }

    if(strcmp(mstring_get_cstr(wifi_mode), CFG_WIFI_MODE_AP) == 0) {
        if(nvs_config_set_wifi_mode(WiFiModeAP) != ESP_OK) {
            error_text = JSON_ERROR("invalid value in [sta_pass]");
            goto err_fail;
        }
    } else {
        if(nvs_config_set_wifi_mode(WiFiModeSTA) != ESP_OK) {
            error_text = JSON_ERROR("invalid value in [sta_pass]");
            goto err_fail;
        }
    }

    httpd_resp_sendstr(req, JSON_RESULT("WIFI settings saved"));
    return ESP_OK;

err_fail:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, error_text);
    free(buffer);
    mstring_free(ap_ssid);
    mstring_free(ap_pass);
    mstring_free(sta_ssid);
    mstring_free(sta_pass);
    mstring_free(wifi_mode);
    return ESP_FAIL;
}

static esp_err_t wifi_list_get_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    uint16_t number = WIFI_SCAN_SIZE;
    wifi_ap_record_t* ap_info = calloc(WIFI_SCAN_SIZE, sizeof(wifi_ap_record_t));
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(wifi_ap_record_t));

    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON* array = cJSON_AddArrayToObject(root, "net_list");

    for(int i = 0; (i < WIFI_SCAN_SIZE) && (i < ap_count); i++) {
        cJSON* object = cJSON_CreateObject();
        cJSON_AddStringToObject(object, "ssid", (const char*)&ap_info[i].ssid);
        cJSON_AddNumberToObject(object, "channel", ap_info[i].primary);
        cJSON_AddNumberToObject(object, "rssi", ap_info[i].rssi);
        cJSON_AddStringToObject(object, "auth", get_auth_mode(ap_info[i].authmode));
        if(ap_info[i].authmode != WIFI_AUTH_WEP) {
            cJSON_AddStringToObject(
                object, "pairwise_cipher", get_pairwise_cipher(ap_info[i].pairwise_cipher));
            cJSON_AddStringToObject(
                object, "group_cipher", get_group_cipher(ap_info[i].group_cipher));
        }
        cJSON_AddItemToArray(array, object);
    }
    cJSON_AddNumberToObject(root, "total", ap_count);
    const char* json_text = cJSON_Print(root);
    httpd_resp_sendstr(req, json_text);
    free((void*)json_text);

    cJSON_Delete(root);
    free(ap_info);
    return ESP_OK;
}

static esp_err_t gpio_led_set_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    int total_length = req->content_len;
    int current_length = 0;
    char* buffer = malloc(256);
    const char* error_text = JSON_ERROR("unknown error");
    int received = 0;
    int32_t led_red = -1;
    int32_t led_green = -1;
    int32_t led_blue = -1;
    httpd_resp_set_type(req, "application/json");

    if(total_length >= 256) {
        error_text = JSON_ERROR("request too long");
        goto err_fail;
    }

    while(current_length < total_length) {
        received = httpd_req_recv(req, buffer + current_length, total_length);
        if(received <= 0) {
            error_text = JSON_ERROR("cannot receive request data");
            goto err_fail;
        }
        current_length += received;
    }
    buffer[total_length] = '\0';

    cJSON* root = cJSON_Parse(buffer);
    cJSON* element;
    element = cJSON_GetObjectItem(root, "red");
    if(element != NULL && element->type == cJSON_Number) {
        led_red = element->valuedouble;
    }

    element = cJSON_GetObjectItem(root, "green");
    if(element != NULL && element->type == cJSON_Number) {
        led_green = element->valuedouble;
    }

    element = cJSON_GetObjectItem(root, "blue");
    if(element != NULL && element->type == cJSON_Number) {
        led_blue = element->valuedouble;
    }
    cJSON_Delete(root);

    if((led_red < 0 && led_green < 0 && led_blue < 0) ||
       (led_red > 255 || led_green > 255 || led_blue > 255)) {
        error_text = JSON_ERROR("[red], [green], [blue] must contain numbers in the range 0-255");
        goto err_fail;
    }

    if(led_red >= 0) {
        led_set_red(led_red);
    }
    if(led_green >= 0) {
        led_set_green(led_green);
    }
    if(led_blue >= 0) {
        led_set_blue(led_blue);
    }

    httpd_resp_sendstr(req, JSON_RESULT("OK"));
    return ESP_OK;

err_fail:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, error_text);
    free(buffer);
    return ESP_FAIL;
}

const httpd_uri_t uri_handlers[] = {

    /*************** SYSTEM ***************/

    {.uri = "/api/v1/system/ping",
     .method = HTTP_GET,
     .handler = system_ping_handler,
     .user_ctx = NULL},

    {.uri = "/api/v1/system/tasks",
     .method = HTTP_GET,
     .handler = system_tasks_handler,
     .user_ctx = NULL},

    {.uri = "/api/v1/system/info",
     .method = HTTP_GET,
     .handler = system_info_get_handler,
     .user_ctx = NULL},

    {.uri = "/api/v1/system/reboot",
     .method = HTTP_POST,
     .handler = system_reboot,
     .user_ctx = NULL},

    /*************** GPIO ***************/

    {.uri = "/api/v1/gpio/led",
     .method = HTTP_POST,
     .handler = gpio_led_set_handler,
     .user_ctx = NULL},

    /*************** WIFI ***************/

    {.uri = "/api/v1/wifi/list",
     .method = HTTP_GET,
     .handler = wifi_list_get_handler,
     .user_ctx = NULL},

    {.uri = "/api/v1/wifi/set_credentials",
     .method = HTTP_POST,
     .handler = wifi_set_credentials_handler,
     .user_ctx = NULL},

    {.uri = "/api/v1/wifi/get_credentials",
     .method = HTTP_GET,
     .handler = wifi_get_credentials_handler,
     .user_ctx = NULL},

    /*************** HTTP ***************/

    {.uri = "/*", .method = HTTP_GET, .handler = http_common_get_handler, .user_ctx = NULL},
};

void network_http_server_init(void) {
    ESP_LOGI(TAG, "init rest server");

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = COUNT_OF(uri_handlers);
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "starting http server");
    if(httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "start http server failed");
        return;
    }

    for(size_t i = 0; i < COUNT_OF(uri_handlers); i++) {
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_handlers[i]));
    }

    ESP_LOGI(TAG, "init rest server done");
}
