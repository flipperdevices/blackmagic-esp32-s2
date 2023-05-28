#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"
#include <gdb-glue.h>
#include <esp_mac.h>
#include <esp_system.h>
#include "esp_chip_info.h"

void cli_device_info(Cli* cli, mstring_t* args) {
    uint8_t mac_addr[8] = {0};

    if(esp_read_mac(mac_addr, ESP_MAC_WIFI_STA) == ESP_OK) {
        cli_printf(
            cli,
            "mac_wifi_sta:            %02x%02x%02x%02x%02x%02x",
            mac_addr[0],
            mac_addr[1],
            mac_addr[2],
            mac_addr[3],
            mac_addr[4],
            mac_addr[5]);
        cli_write_eol(cli);
    }

    if(esp_read_mac(mac_addr, ESP_MAC_WIFI_SOFTAP) == ESP_OK) {
        cli_printf(
            cli,
            "mac_wifi_ap:             %02x%02x%02x%02x%02x%02x",
            mac_addr[0],
            mac_addr[1],
            mac_addr[2],
            mac_addr[3],
            mac_addr[4],
            mac_addr[5]);
        cli_write_eol(cli);
    }

    if(esp_read_mac(mac_addr, ESP_MAC_BT) == ESP_OK) {
        cli_printf(
            cli,
            "mac_bt:                  %02x%02x%02x%02x%02x%02x",
            mac_addr[0],
            mac_addr[1],
            mac_addr[2],
            mac_addr[3],
            mac_addr[4],
            mac_addr[5]);
        cli_write_eol(cli);
    }

    if(esp_read_mac(mac_addr, ESP_MAC_ETH) == ESP_OK) {
        cli_printf(
            cli,
            "mac_eth:                 %02x%02x%02x%02x%02x%02x",
            mac_addr[0],
            mac_addr[1],
            mac_addr[2],
            mac_addr[3],
            mac_addr[4],
            mac_addr[5]);
        cli_write_eol(cli);
    }

    if(esp_read_mac(mac_addr, ESP_MAC_IEEE802154) == ESP_OK) {
        cli_printf(
            cli,
            "mac_IEEE802154:          %02x%02x%02x%02x%02x%02x%02x%02x",
            mac_addr[0],
            mac_addr[1],
            mac_addr[2],
            mac_addr[3],
            mac_addr[4],
            mac_addr[5],
            mac_addr[6],
            mac_addr[7]);
        cli_write_eol(cli);
    }

    cli_printf(cli, "idf_version:             %s", IDF_VER);
    cli_write_eol(cli);

    cli_printf(cli, "blackmagic_version:      %s", gdb_glue_get_bm_version());
    cli_write_eol(cli);

    cli_printf(cli, "firmware_commit:         %s", FW_GIT_COMMIT);
    cli_write_eol(cli);

    cli_printf(cli, "firmware_branch:         %s", FW_GIT_BRANCH);
    cli_write_eol(cli);

    cli_printf(cli, "firmware_branch_num:     %s", FW_GIT_BRANCH_NUM);
    cli_write_eol(cli);

    cli_printf(cli, "firmware_version:        %s", FW_GIT_VERSION);
    cli_write_eol(cli);

    cli_printf(cli, "firmware_build_date:     %s", FW_BUILD_DATE);
    cli_write_eol(cli);

    cli_write_str(cli, "chip_model:              ");
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    switch(chip_info.model) {
    case CHIP_ESP32:
        cli_write_str(cli, "ESP32");
        break;
    case CHIP_ESP32S2:
        cli_write_str(cli, "ESP32-S2");
        break;
    case CHIP_ESP32H2:
        cli_write_str(cli, "ESP32-H2");
        break;
    case CHIP_ESP32S3:
        cli_write_str(cli, "ESP32-S3");
        break;
    case CHIP_ESP32C3:
        cli_write_str(cli, "ESP32-C3");
        break;
    default:
        cli_write_str(cli, "UNKNOWN");
        break;
    }
    cli_write_eol(cli);

    cli_printf(cli, "chip_cores:              %d", chip_info.cores);
    cli_write_eol(cli);

    cli_printf(cli, "chip_revision:           %d", chip_info.revision);
    cli_write_eol(cli);

    cli_printf(
        cli,
        "chip_feature_emb_flash:  %s",
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "true" : "false");
    cli_write_eol(cli);

    cli_printf(
        cli,
        "chip_feature_wifi_bgn:   %s",
        (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "true" : "false");
    cli_write_eol(cli);

    cli_printf(
        cli,
        "chip_feature_ble:        %s",
        (chip_info.features & CHIP_FEATURE_BLE) ? "true" : "false");
    cli_write_eol(cli);

    cli_printf(
        cli,
        "chip_feature_bt:         %s",
        (chip_info.features & CHIP_FEATURE_BT) ? "true" : "false");
    cli_write_eol(cli);

    cli_printf(
        cli,
        "chip_feature_IEEE802154: %s",
        (chip_info.features & CHIP_FEATURE_IEEE802154) ? "true" : "false");
}
