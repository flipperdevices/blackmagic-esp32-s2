#include "nvs.h"
#include "network.h"
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include <string.h>
#include <m-string.h>
#include <mdns.h>
#include <lwip/apps/netbiosns.h>

#define TAG "network"

#define ESP_WIFI_SSID "blackmagic"
#define ESP_WIFI_PASS "iamwitcher"
#define ESP_WIFI_CHANNEL 1

#define MDNS_HOST_NAME "blackmagic"
#define MDNS_INSTANCE "blackmagic web server"

#ifndef min
#define min(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

#define WIFI_MAXIMUM_RETRY 3

// TODO remove global vars
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static WIFIMode wifi_mode = WIFIModeSTA;
static int s_retry_num = 0;

uint32_t network_get_ip(void) {
    tcpip_adapter_ip_info_t ip_info;
    if(wifi_mode == WIFIModeSTA) {
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
    } else {
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
    }

    return ip_info.ip.addr;
}

static void
    sta_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if(s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void
    ap_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if(event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if(event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

static void network_start_ap(mstring_t* ap_ssid, mstring_t* ap_pass) {
    ESP_LOGI(TAG, "init access point mode");
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &ap_event_handler, NULL, NULL));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    wifi_config.ap.ssid_len = strlen("");
    wifi_config.ap.channel = ESP_WIFI_CHANNEL;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    memcpy(
        &wifi_config.ap.ssid,
        mstring_get_cstr(ap_ssid),
        min(sizeof(wifi_config.ap.ssid), mstring_length(ap_ssid)));

    memcpy(
        &wifi_config.ap.password,
        mstring_get_cstr(ap_pass),
        min(sizeof(wifi_config.ap.password), mstring_length(ap_pass)));

    wifi_config.ap.ssid_len = mstring_length(ap_ssid);

    if(mstring_length(ap_pass) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(
        TAG,
        "wifi_init_softap finished. SSID:%s password:%s channel:%d",
        mstring_get_cstr(ap_ssid),
        mstring_get_cstr(ap_pass),
        ESP_WIFI_CHANNEL);

    ESP_LOGI(TAG, "init access point mode done");
}

static bool network_connect_ap(mstring_t* ap_ssid, mstring_t* ap_pass) {
    bool result = false;

    ESP_LOGI(TAG, "init connect to AP");

    wifi_event_group = xEventGroupCreate();
    esp_netif_t* netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &sta_event_handler, NULL));
    ESP_ERROR_CHECK(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_event_handler, NULL));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    memcpy(
        &wifi_config.sta.ssid,
        mstring_get_cstr(ap_ssid),
        min(sizeof(wifi_config.sta.ssid), mstring_length(ap_ssid)));
    memcpy(
        &wifi_config.sta.password,
        mstring_get_cstr(ap_pass),
        min(sizeof(wifi_config.sta.password), mstring_length(ap_pass)));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(
        TAG,
        "init connect to AP done. SSID:%s password:%s",
        mstring_get_cstr(ap_ssid),
        mstring_get_cstr(ap_pass));

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if(bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to AP");
        result = true;
    } else if(bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "failed to connect to AP");
    } else {
        ESP_LOGE(TAG, "unexpected event while connecting to AP");
    }

    ESP_ERROR_CHECK(
        esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_event_handler));
    ESP_ERROR_CHECK(
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &sta_event_handler));
    vEventGroupDelete(wifi_event_group);

    if(!result) {
        ESP_LOGI(TAG, "wifi deinit");
        if(esp_wifi_disconnect() == ESP_FAIL) {
            ESP_LOGE(TAG, "cannot disconnect wifi, internal WIFI error");
            abort();
        }

        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_deinit());
        ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(netif));
        esp_netif_destroy(netif);
        ESP_LOGI(TAG, "wifi deinit done");
    }

    return result;
}

WIFIMode network_init(void) {
    mstring_t* ap_mode = mstring_alloc();
    mstring_t* ap_ssid = mstring_alloc();
    mstring_t* ap_pass = mstring_alloc();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());

    if(nvs_load_string("ap_mode", ap_mode) != ESP_OK) {
        ESP_LOGW(TAG, "cannot load [ap_mode]");
        mstring_set(ap_mode, ESP_WIFI_MODE_AP);
        nvs_save_string("ap_mode", ap_mode);
    }

    if(nvs_load_string("ap_ssid", ap_ssid) != ESP_OK) {
        ESP_LOGW(TAG, "cannot load [ap_ssid]");
        mstring_set(ap_ssid, ESP_WIFI_SSID);
        nvs_save_string("ap_ssid", ap_ssid);
    }

    if(nvs_load_string("ap_pass", ap_pass) != ESP_OK) {
        ESP_LOGW(TAG, "cannot load [ap_pass]");
        mstring_set(ap_pass, ESP_WIFI_PASS);
        nvs_save_string("ap_pass", ap_pass);
    }

    if(mstring_length(ap_pass) < 8) {
        ESP_LOGW(TAG, "too short [ap_pass]");
        mstring_set(ap_ssid, ESP_WIFI_SSID);
        nvs_save_string("ap_ssid", ap_ssid);
        mstring_set(ap_pass, ESP_WIFI_PASS);
        nvs_save_string("ap_pass", ap_pass);
        mstring_set(ap_mode, ESP_WIFI_MODE_AP);
        nvs_save_string("ap_mode", ap_mode);
    }

    if(mstring_length(ap_ssid) < 1 || mstring_length(ap_ssid) > 32) {
        ESP_LOGW(TAG, "too short or too long [ap_ssid]");
        mstring_set(ap_ssid, ESP_WIFI_SSID);
        nvs_save_string("ap_ssid", ap_ssid);
        mstring_set(ap_pass, ESP_WIFI_PASS);
        nvs_save_string("ap_pass", ap_pass);
        mstring_set(ap_mode, ESP_WIFI_MODE_AP);
        nvs_save_string("ap_mode", ap_mode);
    }

    if(strcmp(mstring_get_cstr(ap_mode), ESP_WIFI_MODE_STA) == 0) {
        if(!network_connect_ap(ap_ssid, ap_pass)) {
            mstring_set(ap_ssid, ESP_WIFI_SSID);
            nvs_save_string("ap_ssid", ap_ssid);
            mstring_set(ap_pass, ESP_WIFI_PASS);
            nvs_save_string("ap_pass", ap_pass);
            mstring_set(ap_mode, ESP_WIFI_MODE_AP);
            nvs_save_string("ap_mode", ap_mode);

            ESP_LOGW(TAG, "cannot connect to AP");
            network_start_ap(ap_ssid, ap_pass);
            wifi_mode = WIFIModeAP;
        } else {
            wifi_mode = WIFIModeSTA;
        }
    } else {
        network_start_ap(ap_ssid, ap_pass);
        wifi_mode = WIFIModeAP;
    }

    mstring_free(ap_ssid);
    mstring_free(ap_pass);

    ESP_LOGI(TAG, "init mdns");
    mdns_init();
    mdns_hostname_set(MDNS_HOST_NAME);
    mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {{"board", "esp32"}, {"path", "/"}};

    ESP_ERROR_CHECK(mdns_service_add(
        "ESP32-WebServer",
        "_http",
        "_tcp",
        80,
        serviceTxtData,
        sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));

    ESP_LOGI(TAG, "init MDNS done");

    ESP_LOGI(TAG, "init netbios");
    netbiosns_init();
    netbiosns_set_name(MDNS_HOST_NAME);
    ESP_LOGI(TAG, "init netbios done");

    return wifi_mode;
}

WIFIMode network_get_mode(void) {
    return wifi_mode;
}