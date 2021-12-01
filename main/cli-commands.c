#include <stdlib.h>
#include <stdint.h>
#include <esp_mac.h>
#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"

void cli_id(Cli* cli, mstring_t* args);
void cli_help(Cli* cli, mstring_t* args);
void cli_version(Cli* cli, mstring_t* args);
void cli_gpio_set(Cli* cli, mstring_t* args);

const CliItem cli_items[] = {
    {
        .name = "?",
        .callback = cli_help,
    },
    {
        .name = "id",
        .callback = cli_id,
    },
    {
        .name = "help",
        .callback = cli_help,
    },
    {
        .name = "version",
        .callback = cli_version,
    },
    {
        .name = "gpio_set",
        .callback = cli_gpio_set,
    },
};

size_t cli_items_count = COUNT_OF(cli_items);

void cli_help(Cli* cli, mstring_t* args) {
    for(size_t i = 0; i < cli_items_count; i++) {
        cli_write_str(cli, cli_items[i].name);

        if((i + 1) < cli_items_count) {
            cli_write_eol(cli);
        }
    }
}

void cli_id(Cli* cli, mstring_t* args) {
    uint8_t mac_addr[6] = {0};
    ESP_ERROR_CHECK(esp_read_mac(mac_addr, ESP_MAC_WIFI_STA));
    cli_printf(
        cli,
        "%02X%02X%02X%02X%02X%02X",
        mac_addr[0],
        mac_addr[1],
        mac_addr[2],
        mac_addr[3],
        mac_addr[4],
        mac_addr[5]);
}

void cli_version(Cli* cli, mstring_t* args) {
    cli_printf(cli, "IDF version: %s", IDF_VER);
    cli_write_eol(cli);
    cli_printf(cli, "FW version: %s", "0.1");
}