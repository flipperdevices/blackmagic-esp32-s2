#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <esp_system.h>
#include "cli.h"
#include "led.h"
#include "nvs.h"
#include "helpers.h"
#include "cli-args.h"
#include "cli-commands.h"

void cli_device_info(Cli* cli, mstring_t* args);
void cli_factory_reset(Cli* cli, mstring_t* args);
void cli_gpio_get(Cli* cli, mstring_t* args);
void cli_gpio_set(Cli* cli, mstring_t* args);
void cli_led(Cli* cli, mstring_t* args);
void cli_help(Cli* cli, mstring_t* args);
void cli_ping(Cli* cli, mstring_t* args);
void cli_sw_reboot(Cli* cli, mstring_t* args);
void cli_wifi_scan(Cli* cli, mstring_t* args);
void cli_wifi_ap_clients(Cli* cli, mstring_t* args);
void cli_wifi_ip(Cli* cli, mstring_t* args);
void cli_wifi_sta_info(Cli* cli, mstring_t* args);

void cli_config_get(Cli* cli, mstring_t* args);
void cli_config_set_wifi_mode(Cli* cli, mstring_t* args);
void cli_config_set_ap_pass(Cli* cli, mstring_t* args);
void cli_config_set_ap_ssid(Cli* cli, mstring_t* args);
void cli_config_set_sta_pass(Cli* cli, mstring_t* args);
void cli_config_set_sta_ssid(Cli* cli, mstring_t* args);
void cli_config_set_hostname(Cli* cli, mstring_t* args);

void cli_nvs_dump(Cli* cli, mstring_t* args);

const CliItem cli_items[] = {
    {
        .name = "!",
        .desc = "alias for device_info",
        .callback = cli_device_info,
    },
    {
        .name = "?",
        .desc = "alias for help",
        .callback = cli_help,
    },
    {
        .name = "config_get",
        .desc = "show current config",
        .callback = cli_config_get,
    },
    {
        .name = "config_set_wifi_mode",
        .desc =
            "set Wi-Fi mode, AP (own access point) or STA (join another network), requires a reboot to apply",
        .callback = cli_config_set_wifi_mode,
    },
    {
        .name = "config_set_ap_pass",
        .desc = "set AP mode password, requires a reboot to apply",
        .callback = cli_config_set_ap_pass,
    },
    {
        .name = "config_set_ap_ssid",
        .desc = "set AP mode SSID, requires a reboot to apply",
        .callback = cli_config_set_ap_ssid,
    },
    {
        .name = "config_set_sta_pass",
        .desc = "set STA mode password, requires a reboot to apply",
        .callback = cli_config_set_sta_pass,
    },
    {
        .name = "config_set_sta_ssid",
        .desc = "set STA mode SSID, requires a reboot to apply",
        .callback = cli_config_set_sta_ssid,
    },
    {
        .name = "config_set_hostname",
        .desc = "set MDNS host name, requires a reboot to apply",
        .callback = cli_config_set_hostname,
    },
    {
        .name = "device_info",
        .desc = "show device info (mac, fw version, chip info, etc)",
        .callback = cli_device_info,
    },
    {
        .name = "factory_reset",
        .desc = "reset config (clears NVS storage)",
        .callback = cli_factory_reset,
    },
    {
        .name = "gpio_get",
        .desc = "get gpio level",
        .callback = cli_gpio_get,
    },
    {
        .name = "gpio_set",
        .desc = "set gpio level",
        .callback = cli_gpio_set,
    },
    {
        .name = "help",
        .desc = "show this help",
        .callback = cli_help,
    },
    {
        .name = "led",
        .desc = "set led color",
        .callback = cli_led,
    },
    {
        .name = "nvs_dump",
        .desc = "show all NVS contents",
        .callback = cli_nvs_dump,
    },
    {
        .name = "ping",
        .desc = "answers pong",
        .callback = cli_ping,
    },
    {
        .name = "reboot",
        .desc = "reboot device",
        .callback = cli_sw_reboot,
    },
    {
        .name = "wifi_ap_clients",
        .desc = "list AP mode clients",
        .callback = cli_wifi_ap_clients,
    },
    {
        .name = "wifi_ip",
        .desc = "show current ip, mask and gateway",
        .callback = cli_wifi_ip,
    },
    {
        .name = "wifi_sta_info",
        .desc = "show STA mode network info",
        .callback = cli_wifi_sta_info,
    },
    {
        .name = "wifi_scan",
        .desc = "scan all available Wi-Fi networks (takes some time)",
        .callback = cli_wifi_scan,
    },
};

size_t cli_items_count = COUNT_OF(cli_items);

void cli_help(Cli* cli, mstring_t* args) {
    size_t max_len = 0;
    for(size_t i = 0; i < cli_items_count; i++) {
        if(strlen(cli_items[i].name) > max_len) {
            max_len = strlen(cli_items[i].name);
        }
    }

    max_len += 1;

    for(size_t i = 0; i < cli_items_count; i++) {
        cli_write_str(cli, cli_items[i].name);

        if(cli_items[i].desc != NULL && strlen(cli_items[i].desc)) {
            for(size_t s = 0; s < (max_len - strlen(cli_items[i].name)); s++) {
                cli_write_str(cli, " ");
            }
            cli_write_str(cli, "- ");
            cli_write_str(cli, cli_items[i].desc);
        }

        if((i + 1) < cli_items_count) {
            cli_write_eol(cli);
        }
    }
}

void cli_sw_reboot(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "SW REBOOT");
    cli_write_eol(cli);
    cli_flush(cli);
    esp_restart();
    cli_write_str(cli, "OK");
}

void cli_factory_reset(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "Erasing NVS");
    cli_write_eol(cli);
    cli_flush(cli);
    nvs_erase();
    cli_write_str(cli, "OK");
}

void cli_ping(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "pong");
}

void cli_led_print_usage(Cli* cli) {
    cli_write_str(cli, "Usage: led <red> <green> <blue>");
}

void cli_led(Cli* cli, mstring_t* args) {
    int red;
    int green;
    int blue;
    do {
        if(!cli_args_read_int_and_trim(args, &red)) {
            cli_led_print_usage(cli);
            break;
        }

        if(!cli_args_read_int_and_trim(args, &green)) {
            cli_led_print_usage(cli);
            break;
        }

        if(!cli_args_read_int_and_trim(args, &blue)) {
            cli_led_print_usage(cli);
            break;
        }

        if(red > 255 || red < 0 || green > 255 || green < 0 || blue > 255 || blue < 0) {
            cli_led_print_usage(cli);
            break;
        }

        led_set((uint8_t)red, (uint8_t)green, (uint8_t)blue);
        cli_write_str(cli, "OK");
    } while(false);
}
