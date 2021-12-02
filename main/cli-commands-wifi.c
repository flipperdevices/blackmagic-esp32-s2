#include <string.h>
#include <esp_wifi.h>
#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"

#define WIFI_SCAN_SIZE 20

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

void cli_wifi_scan(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "Scanning...");
    cli_write_eol(cli);

    uint16_t number = WIFI_SCAN_SIZE;
    wifi_ap_record_t* ap_info = calloc(WIFI_SCAN_SIZE, sizeof(wifi_ap_record_t));
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(wifi_ap_record_t));

    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    for(int i = 0; (i < WIFI_SCAN_SIZE) && (i < ap_count); i++) {
        cli_printf(cli, "\"%s\" ", ap_info[i].ssid);
        cli_printf(cli, "%dch ", ap_info[i].primary);
        cli_printf(cli, "%idBm ", ap_info[i].rssi);
        cli_printf(cli, "%s ", get_auth_mode(ap_info[i].authmode));
        if(ap_info[i].authmode != WIFI_AUTH_WEP) {
            cli_printf(cli, "%s ", get_pairwise_cipher(ap_info[i].pairwise_cipher));
            cli_printf(cli, "%s ", get_group_cipher(ap_info[i].group_cipher));
        }
        cli_write_eol(cli);
    }

    cli_write_str(cli, "OK");
    free(ap_info);
}