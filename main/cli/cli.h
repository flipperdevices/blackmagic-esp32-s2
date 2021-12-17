/**
 * @file cli.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-01
 * 
 * Cli commands
 */

#pragma once
#include "stdint.h"
#include "stdlib.h"

typedef struct Cli Cli;
typedef struct CliItem CliItem;

typedef void (*CliWrite)(const uint8_t* data, size_t data_size, void* context);
typedef void (*CliFlush)(void* context);

Cli* cli_init(void);

void cli_set_context(Cli* cli, void* context);

void cli_set_write_cb(Cli* cli, CliWrite write_cb);

void cli_set_flush_cb(Cli* cli, CliFlush flush_cb);

void cli_write(Cli* cli, const uint8_t* data, size_t data_size);

void cli_flush(Cli* cli);

void cli_write_str(Cli* cli, const char* str);

void cli_write_char(Cli* cli, uint8_t c);

void cli_handle_char(Cli* cli, uint8_t c);

void cli_write_eol(Cli* cli);

void cli_printf(Cli* cli, char* format, ...);

void cli_force_motd(Cli* cli);