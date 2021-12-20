#include <m-string.h>
#include <stdbool.h>
#include <string.h>
#include "cli.h"
#include "delay.h"
#include "helpers.h"
#include "cli-commands.h"
#include <esp_log.h>

typedef enum {
    CliSymbolAsciiSOH = 0x01,
    CliSymbolAsciiETX = 0x03,
    CliSymbolAsciiEOT = 0x04,
    CliSymbolAsciiBell = 0x07,
    CliSymbolAsciiBackspace = 0x08,
    CliSymbolAsciiTab = 0x09,
    CliSymbolAsciiCR = 0x0D,
    CliSymbolAsciiEsc = 0x1B,
    CliSymbolAsciiUS = 0x1F,
    CliSymbolAsciiSpace = 0x20,
    CliSymbolAsciiDel = 0x7F,
} CliSymbols;

struct Cli {
    mstring_t* line;
    mstring_t* prev_line;
    size_t cursor_position;
    bool esc_mode;

    void* context;
    CliWrite write_cb;
    CliFlush flush_cb;
};

Cli* cli_init(void) {
    Cli* cli = malloc(sizeof(Cli));
    cli->line = mstring_alloc();
    cli->prev_line = mstring_alloc();
    cli->context = NULL;
    cli->write_cb = NULL;
    cli->flush_cb = NULL;
    cli->esc_mode = false;

    return cli;
}

void cli_reset(Cli* cli) {
    mstring_reset(cli->line);
    cli->cursor_position = 0;
}

void cli_set_context(Cli* cli, void* context) {
    cli->context = context;
}

void cli_set_write_cb(Cli* cli, CliWrite write_cb) {
    cli->write_cb = write_cb;
}

void cli_set_flush_cb(Cli* cli, CliFlush flush_cb) {
    cli->flush_cb = flush_cb;
}

void cli_write(Cli* cli, const uint8_t* data, size_t data_size) {
    if(cli->write_cb != NULL) {
        cli->write_cb(data, data_size, cli->context);
    }
}

void cli_flush(Cli* cli) {
    if(cli->flush_cb != NULL) {
        cli->flush_cb(cli->context);
    }
}

void cli_write_str(Cli* cli, const char* str) {
    cli_write(cli, (const uint8_t*)str, strlen(str));
}

void cli_write_char(Cli* cli, uint8_t c) {
    cli_write(cli, &c, 1);
}

void cli_write_eol(Cli* cli) {
    cli_write_str(cli, "\r\n");
}

void cli_printf(Cli* cli, char* format, ...) {
    mstring_t* str = mstring_alloc();
    va_list args;
    va_start(args, format);
    mstring_vprintf(str, format, args);
    va_end(args);
    cli_write_str(cli, mstring_get_cstr(str));
    mstring_free(str);
}

static void cli_write_motd(Cli* cli) {
    cli_write_str(cli, "                          \r\n");
    cli_write_str(cli, "         ____       BLACK \r\n");
    cli_write_str(cli, "        /   /\\      MAGIC \r\n");
    cli_write_str(cli, "       /   /\\ \\          \r\n");
    cli_write_str(cli, "      (   /  \\ \\     '   \r\n");
    cli_write_str(cli, "     _/  (___/ /_    . '  \r\n");
    cli_write_str(cli, "    ( _____._.__ )   ,'o  \r\n");
    cli_write_str(cli, "    //  /  'o'  |    O .  \r\n");
    cli_write_str(cli, "   //  /  '   ' |\\   ,,,  \r\n");
    cli_write_str(cli, "  //  /\\ '     '| \\_/''/  \r\n");
    cli_write_str(cli, " //  /  )______/\\   \\_/   \r\n");
    cli_write_str(cli, "//   \\ (       ) `---'    \r\n");
    cli_write_str(cli, "\\\\___/  |      \\          \r\n");
    cli_write_str(cli, " \\(  \\  |       \\         \r\n");
    cli_write_str(cli, " / \\_@  |        )        \r\n");
    cli_write_str(cli, " \\    , |        \\        \r\n");
    cli_write_str(cli, " / / /  |        /        \r\n");
    cli_write_str(cli, " \\ \\    :    /\\ /         \r\n");
    cli_write_str(cli, "  \\ \\ \\  :  / |\\)         \r\n");
    cli_write_str(cli, "   \\  /  |\\/| |           \r\n");
    cli_write_str(cli, "   / /   |  |-            \r\n");
    cli_write_str(cli, "   \\    /|__|             \r\n");
    cli_write_str(cli, "    \\  / |''|             \r\n");
    cli_write_str(cli, "     \\ \\  --              \r\n");
    cli_write_str(cli, "      \\/                  \r\n");
}

static void cli_write_prompt(Cli* cli) {
    cli_write_str(cli, ">: ");
}

void cli_force_motd(Cli* cli) {
    cli_write_motd(cli);
    cli_write_eol(cli);
    cli_write_prompt(cli);
    cli_flush(cli);
}

static const CliItem* cli_search_item(Cli* cli, const mstring_t* command) {
    const CliItem* item = NULL;

    for(size_t i = 0; i < cli_items_count; i++) {
        if(mstring_cmp_cstr(command, cli_items[i].name) == 0) {
            item = &cli_items[i];
        }
    }

    return item;
}

static bool cli_handle_enter(Cli* cli) {
    mstring_t* command = mstring_alloc();
    mstring_t* args = mstring_alloc();
    bool result = false;
    const CliItem* item = NULL;

    mstring_set(cli->prev_line, mstring_get_cstr(cli->line));
    mstring_strim(cli->line, "  \n\r\t");

    size_t ws = mstring_search_char(cli->line, ' ', 0);
    if(ws == STRING_FAILURE) {
        mstring_set(command, mstring_get_cstr(cli->line));
    } else {
        mstring_set_n(command, cli->line, 0, ws);
        mstring_set_n(args, cli->line, ws, mstring_size(cli->line));
        mstring_strim(args, "  \n\r\t");
    }

    item = cli_search_item(cli, command);
    if(item != NULL) {
        item->callback(cli, args);
    } else {
        cli_write_str(cli, "Not found");
    }

    mstring_free(command);
    mstring_free(args);
    return result;
}

static void cli_handle_backspace(Cli* cli) {
    if(mstring_size(cli->line) > 0) {
        // Other side
        cli_write_str(cli, "\e[D\e[1P");
        // Our side
        mstring_set_strn(cli->line, mstring_get_cstr(cli->line), mstring_size(cli->line) - 1);
        cli->cursor_position--;
    } else {
        cli_write_char(cli, CliSymbolAsciiBell);
    }
}

void cli_handle_char(Cli* cli, uint8_t c) {
    if(cli->esc_mode) {
        switch(c) {
        case '[':
            cli->esc_mode = true;
            break;
        case 'A':
            if(mstring_size(cli->line) == 0 && mstring_cmp(cli->line, cli->prev_line) != 0) {
                // Set line buffer and cursor position
                mstring_set(cli->line, mstring_get_cstr(cli->prev_line));
                cli->cursor_position = mstring_size(cli->line);
                // Show new line to user
                cli_write_str(cli, mstring_get_cstr(cli->line));
            }
            cli->esc_mode = false;
            break;
        default:
            cli->esc_mode = false;
            break;
        }
    } else {
        switch(c) {
        case CliSymbolAsciiEsc:
            cli->esc_mode = true;
            break;
        case CliSymbolAsciiCR:
            if(mstring_size(cli->line) == 0) {
                cli_write_eol(cli);
            } else {
                cli_write_eol(cli);
                cli_handle_enter(cli);
                cli_reset(cli);
                cli_write_eol(cli);
            }
            cli_write_prompt(cli);
            break;
        case CliSymbolAsciiDel:
        case CliSymbolAsciiBackspace:
            cli_handle_backspace(cli);
            break;
        case CliSymbolAsciiSOH:
            delay(33);
            cli_force_motd(cli);
            break;
        case CliSymbolAsciiETX:
            cli_reset(cli);
            cli_write_eol(cli);
            cli_write_prompt(cli);
            break;
        case CliSymbolAsciiEOT:
            cli_reset(cli);
            break;
        case ' ' ... '~':
            if(cli->cursor_position == mstring_size(cli->line)) {
                mstring_push_back(cli->line, c);
                cli_write_char(cli, c);
            } else {
                mstring_push_back(cli->line, c);
                cli_write_str(cli, "\e[4h");
                cli_write_char(cli, c);
                cli_write_str(cli, "\e[4l");
            }
            break;
        default:
            break;
        }
    }

    cli_flush(cli);
}
