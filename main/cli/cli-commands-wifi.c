#include <string.h>
#include <esp_wifi.h>
#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"
#include "network.h"

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

void cli_wifi_scan(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "Scanning...");
    cli_write_eol(cli);
    cli_flush(cli);

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

void cli_wifi_ap_clients(Cli* cli, mstring_t* args) {
    wifi_sta_list_t* wifi_sta_list = malloc(sizeof(wifi_sta_list_t));
    esp_err_t err = esp_wifi_ap_get_sta_list(wifi_sta_list);

    if(err == ESP_OK) {
        for(size_t i = 0; i < wifi_sta_list->num; i++) {
            cli_printf(
                cli,
                "%02x:%02x:%02x:%02x:%02x:%02x ",
                wifi_sta_list->sta[i].mac[0],
                wifi_sta_list->sta[i].mac[1],
                wifi_sta_list->sta[i].mac[2],
                wifi_sta_list->sta[i].mac[3],
                wifi_sta_list->sta[i].mac[4],
                wifi_sta_list->sta[i].mac[5]);
            cli_printf(cli, "%idBm ", wifi_sta_list->sta[i].rssi);

            if(wifi_sta_list->sta[i].phy_11b) cli_write_str(cli, "b");
            if(wifi_sta_list->sta[i].phy_11g) cli_write_str(cli, "g");
            if(wifi_sta_list->sta[i].phy_11n) cli_write_str(cli, "n");
            if(wifi_sta_list->sta[i].phy_lr) cli_write_str(cli, "lr");
            if(wifi_sta_list->sta[i].is_mesh_child) cli_write_str(cli, "m");

            cli_write_eol(cli);
        }

        cli_write_str(cli, "OK");
    } else {
        cli_write_str(cli, "FAIL");
    }

    free(wifi_sta_list);
}

void cli_wifi_sta_info(Cli* cli, mstring_t* args) {
    wifi_ap_record_t* ap_info = malloc(sizeof(wifi_ap_record_t));
    esp_err_t err = esp_wifi_sta_get_ap_info(ap_info);

    if(err == ESP_OK) {
        cli_printf(
            cli,
            "%02x:%02x:%02x:%02x:%02x:%02x ",
            ap_info->bssid[0],
            ap_info->bssid[1],
            ap_info->bssid[2],
            ap_info->bssid[3],
            ap_info->bssid[4],
            ap_info->bssid[5]);
        cli_printf(cli, "%dch ", ap_info->primary);
        cli_printf(cli, "%idBm ", ap_info->rssi);
        if(ap_info->phy_11b) cli_write_str(cli, "b");
        if(ap_info->phy_11g) cli_write_str(cli, "g");
        if(ap_info->phy_11n) cli_write_str(cli, "n");
        if(ap_info->phy_lr) cli_write_str(cli, "lr");
        cli_write_eol(cli);

        cli_write_str(cli, "OK");
    } else {
        cli_write_str(cli, "FAIL");
    }

    free(ap_info);
}

void cli_wifi_ip(Cli* cli, mstring_t* args) {
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = NULL;
    wifi_mode_t mode;

    if (esp_wifi_get_mode(&mode) == ESP_OK) {
        if (mode == WIFI_MODE_STA) {
            netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        } else if (mode == WIFI_MODE_APSTA) {
            netif = esp_netif_get_handle_from_ifkey("WIFI_AP_STA_DEF");
        }

        if (netif != NULL) {
            esp_netif_get_ip_info(netif, &ip_info);

            cli_printf(cli, "ip:   %d.%d.%d.%d",
                       (ip_info.ip.addr >> 0) & 0xFF,
                       (ip_info.ip.addr >> 8) & 0xFF,
                       (ip_info.ip.addr >> 16) & 0xFF,
                       (ip_info.ip.addr >> 24) & 0xFF);
            cli_write_eol(cli);
            cli_printf(cli, "mask: %d.%d.%d.%d",
                       (ip_info.netmask.addr >> 0) & 0xFF,
                       (ip_info.netmask.addr >> 8) & 0xFF,
                       (ip_info.netmask.addr >> 16) & 0xFF,
                       (ip_info.netmask.addr >> 24) & 0xFF);
            cli_write_eol(cli);
            cli_printf(cli, "gw:   %d.%d.%d.%d",
                       (ip_info.gw.addr >> 0) & 0xFF,
                       (ip_info.gw.addr >> 8) & 0xFF,
                       (ip_info.gw.addr >> 16) & 0xFF,
                       (ip_info.gw.addr >> 24) & 0xFF);
        } else {
            cli_write_str(cli, "FAIL");
        }
    } else {
        cli_write_str(cli, "FAIL");
    }
}

