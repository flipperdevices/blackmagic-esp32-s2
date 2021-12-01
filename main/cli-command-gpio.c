#include <driver/gpio.h>
#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"

typedef struct {
    const char* name;
    const gpio_num_t gpio_num;
} GPIOItem;

#define MAKE_IO(pin_index) \
    { .name = "IO" #pin_index, .gpio_num = GPIO_NUM_##pin_index }

#define MAKE_IO_NAMED(pin_name, pin_index) \
    { .name = pin_name, .gpio_num = GPIO_NUM_##pin_index }

const GPIOItem gpio_items[] = {
    // SWD
    MAKE_IO(1),
    MAKE_IO_NAMED("SWCLK", 1),
    MAKE_IO(2),
    MAKE_IO_NAMED("SWDIO", 2),
    // I2C_PULL_UP
    MAKE_IO(3),
    MAKE_IO_NAMED("I2C_PULL_UP", 3),
    // GPIO
    MAKE_IO(7),
    MAKE_IO(8),
    MAKE_IO(9),
    // SPI
    MAKE_IO(10),
    MAKE_IO_NAMED("SWO", 10),
    MAKE_IO_NAMED("SPI_CS", 10),
    MAKE_IO(11),
    MAKE_IO_NAMED("SPI_MOSI", 11),
    MAKE_IO(12),
    MAKE_IO_NAMED("SPI_CLK", 12),
    MAKE_IO(13),
    MAKE_IO_NAMED("SPI_MISO", 13),
    // GPIO
    MAKE_IO(14),
    MAKE_IO(15),
    MAKE_IO(16),
    // CLI UART
    MAKE_IO(17),
    MAKE_IO_NAMED("CLI_TXD", 17),
    MAKE_IO(18),
    MAKE_IO_NAMED("CLI_RXD", 18),
    // GPIO
    MAKE_IO(21),
    MAKE_IO(33),
    MAKE_IO(34),
    MAKE_IO(35),
    MAKE_IO(36),
    MAKE_IO(37),
    // JTAG
    MAKE_IO(38),
    MAKE_IO_NAMED("JTAG_SRST", 38),
    MAKE_IO(39),
    MAKE_IO_NAMED("JTAG_TCK", 39),
    MAKE_IO(40),
    MAKE_IO_NAMED("JTAG_TDI", 40),
    MAKE_IO(41),
    MAKE_IO_NAMED("JTAG_TDO", 41),
    MAKE_IO(42),
    MAKE_IO_NAMED("JTAG_TMS", 41),
};

size_t gpio_items_count = COUNT_OF(gpio_items);

static void cli_gpio_print_list(Cli* cli) {
    cli_write_str(cli, "pin names: ");
    for(size_t i = 0; i < gpio_items_count; i++) {
        cli_write_str(cli, gpio_items[i].name);
        if((i + 1) < gpio_items_count) {
            cli_write_str(cli, ", ");
        }
    }
}

static gpio_num_t gpio_search(mstring_t* gpio_name) {
    gpio_num_t gpio_num = GPIO_NUM_NC;
    for(size_t i = 0; i < gpio_items_count; i++) {
        if(mstring_cmp_cstr(gpio_name, gpio_items[i].name) == 0) {
            gpio_num = gpio_items[i].gpio_num;
            break;
        }
    }

    return gpio_num;
}

void cli_gpio_set(Cli* cli, mstring_t* args) {
    mstring_t* gpio_name = mstring_alloc();
    int gpio_value = 0;

    do {
        if(!cli_args_read_string_and_trim(args, gpio_name) ||
           !cli_args_read_int_and_trim(args, &gpio_value)) {
            cli_write_str(cli, "Usage: gpio_set <pin_name> <value>");
            cli_write_eol(cli);
            cli_gpio_print_list(cli);
            break;
        }

        gpio_num_t gpio_num = gpio_search(gpio_name);
        if(gpio_num == GPIO_NUM_NC) {
            cli_write_str(cli, "Invalid <pin_name>");
            cli_write_eol(cli);
            cli_gpio_print_list(cli);
            break;
        }

        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1 << gpio_num);
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;
        gpio_config(&io_conf);
        gpio_set_level(gpio_num, gpio_value > 0);

        cli_write_str(cli, "OK");
    } while(false);

    mstring_free(gpio_name);
}