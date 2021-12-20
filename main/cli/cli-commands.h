/**
 * @file cli-commands.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-01
 * 
 * Cli commands list
 */
#pragma once
#include <m-string.h>

typedef void (*CliCallback)(Cli* cli, mstring_t* args);

struct CliItem {
    const char* name;
    const char* desc;
    CliCallback callback;
};

extern const CliItem cli_items[];
extern size_t cli_items_count;