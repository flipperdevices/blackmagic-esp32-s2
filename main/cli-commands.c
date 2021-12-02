#include <stdlib.h>
#include <stdint.h>
#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"

void cli_help(Cli* cli, mstring_t* args);
void cli_gpio_set(Cli* cli, mstring_t* args);
void cli_device_info(Cli* cli, mstring_t* args);

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
        .name = "help",
        .callback = cli_help,
    },
    {
        .name = "gpio_set",
        .callback = cli_gpio_set,
    },
    {
        .name = "device_info",
        .callback = cli_device_info,
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