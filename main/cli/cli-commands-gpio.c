#include <driver/gpio.h>
#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"

typedef struct {
    const char* name;
    const gpio_num_t gpio_num;
    const bool get;
    const bool set;
} GPIOItem;

#define MAKE_IO(pin_index) \
    { .name = "IO" #pin_index, .gpio_num = GPIO_NUM_##pin_index, .get = true, .set = true }

#define MAKE_IO_NAMED(pin_name, pin_index) \
    { .name = pin_name, .gpio_num = GPIO_NUM_##pin_index, .get = true, .set = true }

#define MAKE_IO_READONLY(pin_index) \
    { .name = "IO" #pin_index, .gpio_num = GPIO_NUM_##pin_index, .get = true, .set = false }

#define MAKE_IO_NAMED_READONLY(pin_name, pin_index) \
    { .name = pin_name, .gpio_num = GPIO_NUM_##pin_index, .get = true, .set = false }

static const GPIOItem gpio_items[] = {
    //BOOT
    MAKE_IO_READONLY(0),
    MAKE_IO_NAMED_READONLY("BOOT", 0),
    // SWD
    MAKE_IO(1),
    MAKE_IO_NAMED("SWCLK", 1),
    MAKE_IO(2),
    MAKE_IO_NAMED("SWDIO", 2),
    // I2C_PULL_UP
    MAKE_IO(3),
    MAKE_IO_NAMED("I2C_PULL_UP", 3),
    // I2C_PULL_UP
    MAKE_IO(4),
    MAKE_IO_NAMED("LED_BLUE", 4),
    MAKE_IO(5),
    MAKE_IO_NAMED("LED_GREEN", 5),
    MAKE_IO(6),
    MAKE_IO_NAMED("LED_RED", 6),
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

static void cli_gpio_print_list_set(Cli* cli) {
    cli_write_str(cli, "Pin names: ");
    for(size_t i = 0; i < COUNT_OF(gpio_items); i++) {
        if(gpio_items[i].set) {
            cli_write_str(cli, gpio_items[i].name);
            if((i + 1) < COUNT_OF(gpio_items)) {
                cli_write_str(cli, " ");
            }
        }
    }
}

static gpio_num_t gpio_search_set(mstring_t* gpio_name) {
    gpio_num_t gpio_num = GPIO_NUM_NC;
    for(size_t i = 0; i < COUNT_OF(gpio_items); i++) {
        if(mstring_cmp_cstr(gpio_name, gpio_items[i].name) == 0 && gpio_items[i].set) {
            gpio_num = gpio_items[i].gpio_num;
            break;
        }
    }

    return gpio_num;
}

static void cli_gpio_print_list_get(Cli* cli) {
    cli_write_str(cli, "Pin names: ");
    for(size_t i = 0; i < COUNT_OF(gpio_items); i++) {
        if(gpio_items[i].get) {
            cli_write_str(cli, gpio_items[i].name);
            if((i + 1) < COUNT_OF(gpio_items)) {
                cli_write_str(cli, " ");
            }
        }
    }
}

static gpio_num_t gpio_search_get(mstring_t* gpio_name) {
    gpio_num_t gpio_num = GPIO_NUM_NC;
    for(size_t i = 0; i < COUNT_OF(gpio_items); i++) {
        if(mstring_cmp_cstr(gpio_name, gpio_items[i].name) == 0 && gpio_items[i].get) {
            gpio_num = gpio_items[i].gpio_num;
            break;
        }
    }

    return gpio_num;
}

void cli_gpio_set_print_usage(Cli* cli) {
    cli_write_str(cli, "Usage: gpio_set <pin_names> <value>");
    cli_write_eol(cli);
    cli_gpio_print_list_set(cli);
    cli_write_eol(cli);
    cli_write_str(cli, "Examples:");
    cli_write_eol(cli);
    cli_write_str(cli, "  gpio_set IO8 1");
    cli_write_eol(cli);
    cli_write_str(cli, "  gpio_set IO8 IO10 SPI_MOSI 1");
}

void cli_gpio_get_print_usage(Cli* cli) {
    cli_write_str(cli, "Usage: gpio_get <pin_names>");
    cli_write_eol(cli);
    cli_gpio_print_list_get(cli);
    cli_write_eol(cli);
    cli_write_str(cli, "Examples:");
    cli_write_eol(cli);
    cli_write_str(cli, "  gpio_get IO8");
    cli_write_eol(cli);
    cli_write_str(cli, "  gpio_get IO8 IO10 SPI_MOSI");
}

void cli_gpio_set(Cli* cli, mstring_t* args) {
    mstring_t* gpio_name = mstring_alloc();
    int gpio_value = 0;
    size_t argc = mstring_count_char(args, ' ') + 1;

    do {
        if(argc < 2) {
            cli_gpio_set_print_usage(cli);
            break;
        }

        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = 0;
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;

        bool error = false;

        for(size_t i = 0; i < (argc - 1); i++) {
            if(cli_args_read_string_and_trim(args, gpio_name)) {
                gpio_num_t gpio_num = gpio_search_set(gpio_name);

                if(gpio_num == GPIO_NUM_NC) {
                    cli_printf(cli, "Invalid <pin_name> %s", mstring_get_cstr(gpio_name));
                    cli_write_eol(cli);
                    error = true;
                    break;
                } else {
                    io_conf.pin_bit_mask |= (1LL << (uint64_t)gpio_num);
                }
            } else {
                error = true;
                break;
            }
        }

        if(!cli_args_read_int_and_trim(args, &gpio_value) || error) {
            cli_gpio_set_print_usage(cli);
            break;
        }

        gpio_config(&io_conf);

        for(size_t i = 0; i < 64; i++) {
            if((io_conf.pin_bit_mask & (1LL << (uint64_t)i)) > 0) {
                gpio_set_level(i, (gpio_value > 0));
            }
        }

        cli_write_str(cli, "OK");
    } while(false);

    mstring_free(gpio_name);
}

void cli_gpio_get(Cli* cli, mstring_t* args) {
    mstring_t* gpio_name = mstring_alloc();
    size_t argc = mstring_count_char(args, ' ') + 1;

    do {
        if(argc < 1) {
            cli_gpio_get_print_usage(cli);
            break;
        }

        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = 0;
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;

        bool error = false;
        gpio_num_t last_gpio_num = GPIO_NUM_NC;

        for(size_t i = 0; i < argc; i++) {
            if(cli_args_read_string_and_trim(args, gpio_name)) {
                gpio_num_t gpio_num = gpio_search_get(gpio_name);

                if(gpio_num == GPIO_NUM_NC) {
                    cli_printf(cli, "Invalid <pin_name> %s", mstring_get_cstr(gpio_name));
                    cli_write_eol(cli);
                    error = true;
                    break;
                } else {
                    if(gpio_num <= last_gpio_num) {
                        cli_printf(
                            cli,
                            "<pin_name> %s is less than or equal to the previous",
                            mstring_get_cstr(gpio_name));
                        cli_write_eol(cli);
                        error = true;
                        break;
                    } else {
                        last_gpio_num = gpio_num;
                        io_conf.pin_bit_mask |= (1LL << (uint64_t)gpio_num);
                    }
                }
            } else {
                error = true;
                break;
            }
        }

        if(error) {
            cli_gpio_get_print_usage(cli);
            break;
        }

        gpio_config(&io_conf);

        for(size_t i = 0; i < 64; i++) {
            if((io_conf.pin_bit_mask & (1LL << (uint64_t)i)) > 0) {
                cli_printf(cli, "%d ", gpio_get_level(i));
            }
        }
    } while(false);

    mstring_free(gpio_name);
}
