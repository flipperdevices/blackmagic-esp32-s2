
#include <esp_http_server.h>
#include <svelte-portal.h>
#include <esp_log.h>
#include <cJSON.h>
#include "network.h"
#include "nvs.h"

#define TAG "network-http"
#define JSON_ERROR(error_text) "{\"error\": \"" error_text "\"}"
#define JSON_RESULT(result_text) "{\"result\": \"" result_text "\"}"

#define WIFI_SCAN_SIZE 20
#include "esp_wifi.h"

static const char* get_auth_mode(int authmode) {
    switch(authmode) {
    case WIFI_AUTH_OPEN:
        return "OPEN";
    case WIFI_AUTH_WEP:
        return "WEP";
    case WIFI_AUTH_WPA_PSK:
        return "WPA PSK";
    case WIFI_AUTH_WPA2_PSK:
        return "WPA2 PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
        return "WPA WPA2 PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return "WPA2 ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK:
        return "WPA3 PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return "WPA2 WPA3 PSK";
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

static esp_err_t http_common_get_handler(httpd_req_t* req) {
    const uint8_t* file_start = NULL;
    const uint8_t* file_end = NULL;
    if(strcmp("/", req->uri) == 0 || strcmp("/index.html", req->uri) == 0) {
        file_start = index_html_start;
        file_end = index_html_end;
        httpd_resp_set_type(req, "text/html");
    } else if(strcmp("/build/bundle.css", req->uri) == 0) {
        file_start = build_bundle_css_start;
        file_end = build_bundle_css_end;
        httpd_resp_set_type(req, "text/css");
    } else if(strcmp("/build/bundle.js", req->uri) == 0) {
        file_start = build_bundle_js_start;
        file_end = build_bundle_js_end;
        httpd_resp_set_type(req, "application/javascript");
    } else if(strcmp("/assets/ega8.otf", req->uri) == 0) {
        file_start = assets_ega8_otf_start;
        file_end = assets_ega8_otf_end;
        httpd_resp_set_type(req, "application/x-font-opentype");
    } else if(strcmp("/assets/favicon.ico", req->uri) == 0 || strcmp("/favicon.ico", req->uri) == 0) {
        file_start = assets_favicon_ico_start;
        file_end = assets_favicon_ico_end;
        httpd_resp_set_type(req, "image/x-icon");
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

static esp_err_t system_info_get_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cJSON_AddStringToObject(root, "idf-version", IDF_VER);
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
    }
    cJSON_AddNumberToObject(root, "revision", chip_info.revision);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);

    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_8BIT);
    cJSON* heap = cJSON_AddObjectToObject(root, "heap");
    cJSON_AddNumberToObject(heap, "total_free_bytes", info.total_free_bytes);
    cJSON_AddNumberToObject(heap, "total_allocated_bytes", info.total_allocated_bytes);
    cJSON_AddNumberToObject(heap, "largest_free_block", info.largest_free_block);
    cJSON_AddNumberToObject(heap, "minimum_free_bytes", info.minimum_free_bytes);

    const char* json_text = cJSON_Print(root);
    httpd_resp_sendstr(req, json_text);
    free((void*)json_text);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t wifi_mode_get_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();

    switch(network_get_mode()) {
    case WIFIModeSTA:
        cJSON_AddStringToObject(root, "mode", "STA");
        break;
    case WIFIModeAP:
        cJSON_AddStringToObject(root, "mode", "AP");
        break;
    }

    const char* json_text = cJSON_Print(root);
    httpd_resp_sendstr(req, json_text);
    free((void*)json_text);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t wifi_get_credenitals_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();

    mstring_t* ap_ssid = mstring_alloc();
    mstring_t* ap_pass = mstring_alloc();
    mstring_t* ap_mode = mstring_alloc();

    if(nvs_load_string("ap_ssid", ap_ssid) != ESP_OK) {
        mstring_set(ap_ssid, "");
    }

    if(nvs_load_string("ap_pass", ap_pass) != ESP_OK) {
        mstring_set(ap_pass, "");
    }

    if(nvs_load_string("ap_mode", ap_mode) != ESP_OK) {
        mstring_set(ap_mode, "");
    }

    cJSON_AddStringToObject(root, "ssid", mstring_get_cstr(ap_ssid));
    cJSON_AddStringToObject(root, "pass", mstring_get_cstr(ap_pass));
    cJSON_AddStringToObject(root, "mode", mstring_get_cstr(ap_mode));

    const char* json_text = cJSON_Print(root);
    httpd_resp_sendstr(req, json_text);
    free((void*)json_text);
    cJSON_Delete(root);
    mstring_free(ap_ssid);
    mstring_free(ap_pass);

    return ESP_OK;
}

static esp_err_t wifi_set_credenitals_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    int total_length = req->content_len;
    int cur_len = 0;
    char* buffer = malloc(256);
    mstring_t* ap_ssid = mstring_alloc();
    mstring_t* ap_pass = mstring_alloc();
    mstring_t* ap_mode = mstring_alloc();
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
    if(cJSON_GetObjectItem(root, "ssid") != NULL) {
        mstring_set(ap_ssid, cJSON_GetObjectItem(root, "ssid")->valuestring);
    } else {
        cJSON_Delete(root);
        error_text = JSON_ERROR("request dont have [ssid] field");
        goto err_fail;
    }

    if(cJSON_GetObjectItem(root, "pass") != NULL) {
        mstring_set(ap_pass, cJSON_GetObjectItem(root, "pass")->valuestring);
    } else {
        cJSON_Delete(root);
        error_text = JSON_ERROR("request dont have [pass] field");
        goto err_fail;
    }

    if(cJSON_GetObjectItem(root, "mode") != NULL) {
        mstring_set(ap_mode, cJSON_GetObjectItem(root, "mode")->valuestring);
    } else {
        cJSON_Delete(root);
        error_text = JSON_ERROR("request dont have [mode] field");
        goto err_fail;
    }
    cJSON_Delete(root);

    if(strcmp(mstring_get_cstr(ap_mode), ESP_WIFI_MODE_AP) != 0 &&
       strcmp(mstring_get_cstr(ap_mode), ESP_WIFI_MODE_STA) != 0) {
        error_text = JSON_ERROR("invalid value in [mode]");
        goto err_fail;
    }

    if(mstring_length(ap_pass) < 8) {
        error_text = JSON_ERROR("too short value in [pass]");
        goto err_fail;
    }
    if(mstring_length(ap_ssid) < 1) {
        error_text = JSON_ERROR("too short value in [ssid]");
        goto err_fail;
    }

    if(nvs_save_string("ap_ssid", ap_ssid) != ESP_OK) {
        error_text = JSON_ERROR("cannot save [ap_ssid]");
        goto err_fail;
    }
    if(nvs_save_string("ap_pass", ap_pass) != ESP_OK) {
        error_text = JSON_ERROR("cannot save [ap_pass]");
        goto err_fail;
    }
    if(nvs_save_string("ap_mode", ap_mode) != ESP_OK) {
        error_text = JSON_ERROR("cannot save [ap_mode]");
        goto err_fail;
    }

    httpd_resp_sendstr(req, JSON_RESULT("WIFI settings saved"));
    return ESP_OK;

err_fail:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, error_text);
    free(buffer);
    mstring_free(ap_ssid);
    mstring_free(ap_pass);
    mstring_free(ap_mode);
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

void network_http_server_init(void) {
    ESP_LOGI(TAG, "init rest server");

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "starting http server");
    if(httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "start http server failed");
        return;
    }

    // ping handler
    httpd_uri_t system_ping_uri = {
        .uri = "/api/v1/system/ping",
        .method = HTTP_GET,
        .handler = system_ping_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &system_ping_uri);

    // system info handler
    httpd_uri_t system_info_get_uri = {
        .uri = "/api/v1/system/info",
        .method = HTTP_GET,
        .handler = system_info_get_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &system_info_get_uri);

    // wifi nets handler
    httpd_uri_t wifi_list_get_uri = {
        .uri = "/api/v1/wifi/list",
        .method = HTTP_GET,
        .handler = wifi_list_get_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &wifi_list_get_uri);

    // wifi mode handler
    httpd_uri_t wifi_mode_get_uri = {
        .uri = "/api/v1/wifi/mode",
        .method = HTTP_GET,
        .handler = wifi_mode_get_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &wifi_mode_get_uri);

    // wifi set credenitals handler
    httpd_uri_t wifi_set_credenitals_uri = {
        .uri = "/api/v1/wifi/set_credenitals",
        .method = HTTP_POST,
        .handler = wifi_set_credenitals_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &wifi_set_credenitals_uri);

    // wifi get credenitals handler
    httpd_uri_t wifi_get_credenitals_uri = {
        .uri = "/api/v1/wifi/get_credenitals",
        .method = HTTP_GET,
        .handler = wifi_get_credenitals_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &wifi_get_credenitals_uri);

    // common http handler
    httpd_uri_t common_get_uri = {
        .uri = "/*", .method = HTTP_GET, .handler = http_common_get_handler, .user_ctx = NULL};
    httpd_register_uri_handler(server, &common_get_uri);

    ESP_LOGI(TAG, "init rest server done");
}
