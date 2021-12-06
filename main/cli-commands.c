#include <stdlib.h>
#include <stdint.h>
#include "cli.h"
#include "led.h"
#include "nvs.h"
#include "helpers.h"
#include "cli-args.h"
#include "cli-commands.h"

void cli_led(Cli* cli, mstring_t* args);
void cli_help(Cli* cli, mstring_t* args);
void cli_ping(Cli* cli, mstring_t* args);
void cli_gpio_set(Cli* cli, mstring_t* args);
void cli_gpio_get(Cli* cli, mstring_t* args);
void cli_device_info(Cli* cli, mstring_t* args);
void cli_factory_reset(Cli* cli, mstring_t* args);
void cli_wifi_scan(Cli* cli, mstring_t* args);

const CliItem cli_items[] = {
    {
        .name = "!",
        .callback = cli_device_info,
    },
    {
        .name = "?",
        .callback = cli_help,
    },
    {
        .name = "led",
        .callback = cli_led,
    },
    {
        .name = "help",
        .callback = cli_help,
    },
    {
        .name = "ping",
        .callback = cli_ping,
    },
    {
        .name = "gpio_set",
        .callback = cli_gpio_set,
    },
    {
        .name = "gpio_get",
        .callback = cli_gpio_get,
    },
    {
        .name = "wifi_scan",
        .callback = cli_wifi_scan,
    },
    {
        .name = "device_info",
        .callback = cli_device_info,
    },
    {
        .name = "factory_reset",
        .callback = cli_factory_reset,
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

void cli_factory_reset(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "Erasing NVS");
    cli_write_eol(cli);
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