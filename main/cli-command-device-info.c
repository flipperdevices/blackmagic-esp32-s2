#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"
#include <gdb-glue.h>
#include <esp_mac.h>

void cli_device_info(Cli* cli, mstring_t* args) {
    uint8_t mac_addr[6] = {0};
    ESP_ERROR_CHECK(esp_read_mac(mac_addr, ESP_MAC_WIFI_STA));

    cli_printf(cli, "hardware_uid: ");
    cli_printf(
        cli,
        "%02x%02x%02x%02x%02x%02x",
        mac_addr[0],
        mac_addr[1],
        mac_addr[2],
        mac_addr[3],
        mac_addr[4],
        mac_addr[5]);
    cli_write_eol(cli);

    cli_printf(cli, "idf_ver: %s", IDF_VER);
    cli_write_eol(cli);
    cli_printf(cli, "blackmagic_version: %s", gdb_glue_get_bm_version());
    cli_write_eol(cli);
    cli_printf(cli, "firmware_commit: %s", FW_GIT_COMMIT);
    cli_write_eol(cli);
    cli_printf(cli, "firmware_branch: %s", FW_GIT_BRANCH);
    cli_write_eol(cli);
    cli_printf(cli, "firmware_branch_num: %s", FW_GIT_BRANCH_NUM);
    cli_write_eol(cli);
    cli_printf(cli, "firmware_version: %s", FW_GIT_VERSION);
    cli_write_eol(cli);
    cli_printf(cli, "firmware_build_date: %s", FW_BUILD_DATE);
}