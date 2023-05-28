#include "nvs.h"
#include "nvs-config.h"
#include "network.h"
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_mac.h>
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
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static WiFiMode wifi_mode = WiFiModeSTA;

uint32_t network_get_ip()
{
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = NULL;
    wifi_mode_t wifi_mode;

    esp_wifi_get_mode(&wifi_mode);

    if (wifi_mode == WIFI_MODE_STA) {
        netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    } else {
        netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    }

    esp_netif_get_ip_info(netif, &ip_info);
    return ip_info.ip.addr;
}


static void
    sta_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
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
    wifi_config.ap.ssid_len = 0;
    wifi_config.ap.channel = ESP_WIFI_CHANNEL;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    memcpy(
        &wifi_config.ap.ssid,
        mstring_get_cstr(ap_ssid),
        min(sizeof(wifi_config.ap.ssid), mstring_size(ap_ssid)));

    memcpy(
        &wifi_config.ap.password,
        mstring_get_cstr(ap_pass),
        min(sizeof(wifi_config.ap.password), mstring_size(ap_pass)));

    wifi_config.ap.ssid_len = mstring_size(ap_ssid);

    if(mstring_size(ap_pass) == 0) {
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
    esp_netif_create_default_wifi_sta();

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
        min(sizeof(wifi_config.sta.ssid), mstring_size(ap_ssid)));
    memcpy(
        &wifi_config.sta.password,
        mstring_get_cstr(ap_pass),
        min(sizeof(wifi_config.sta.password), mstring_size(ap_pass)));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(
        TAG,
        "init connect to AP done. SSID:%s password:%s",
        mstring_get_cstr(ap_ssid),
        mstring_get_cstr(ap_pass));

    return result;
}

void network_hostnames_init(void) {
    mstring_t* hostname = mstring_alloc();

    ESP_LOGI(TAG, "init mdns");
    mdns_init();

    nvs_config_get_hostname(hostname);
    mdns_hostname_set(mstring_get_cstr(hostname));
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
    netbiosns_set_name(mstring_get_cstr(hostname));
    ESP_LOGI(TAG, "init netbios done");

    mstring_free(hostname);
}

WiFiMode network_init(void) {
    mstring_t* ssid = mstring_alloc();
    mstring_t* pass = mstring_alloc();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());

    nvs_config_get_wifi_mode(&wifi_mode);

    network_hostnames_init();

    switch(wifi_mode) {
    case WiFiModeAP:
        nvs_config_get_ap_ssid(ssid);
        nvs_config_get_ap_pass(pass);
        network_start_ap(ssid, pass);
        break;
    case WiFiModeSTA:
        nvs_config_get_sta_ssid(ssid);
        nvs_config_get_sta_pass(pass);
        network_connect_ap(ssid, pass);
        break;
    case WiFiModeDisabled:
        break;
    }

    mstring_free(ssid);
    mstring_free(pass);

    return wifi_mode;
}

WiFiMode network_get_mode(void) {
    return wifi_mode;
}
