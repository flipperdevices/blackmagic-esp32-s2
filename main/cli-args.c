#include <stdio.h>
#include "cli-args.h"

size_t cli_args_get_first_word_length(mstring_t* args) {
    size_t ws = mstring_search_char(args, ' ', 0);
    if(ws == STRING_FAILURE) {
        ws = mstring_size(args);
    }

    return ws;
}

size_t cli_args_length(mstring_t* args) {
    return mstring_size(args);
}

bool cli_args_read_int_and_trim(mstring_t* args, int* value) {
    size_t cmd_length = cli_args_get_first_word_length(args);

    if(cmd_length == 0) {
        return false;
    }

    if(sscanf(mstring_get_cstr(args), "%d", value) == 1) {
        mstring_right(args, cmd_length);
        mstring_strim(args, "  \r\n\t");
        return true;
    }

    return false;
}

bool cli_args_read_string_and_trim(mstring_t* args, mstring_t* word) {
    size_t cmd_length = cli_args_get_first_word_length(args);

    if(cmd_length == 0) {
        return false;
    }

    mstring_set_n(word, args, 0, cmd_length);
    mstring_right(args, cmd_length);
    mstring_strim(args, "  \r\n\t");

    return true;
}

bool cli_args_read_quoted_string_and_trim(mstring_t* args, mstring_t* word) {
    if(mstring_size(args) < 2 || mstring_get_char(args, 0) != '\"') {
        return false;
    }

    size_t second_quote_pos = mstring_search_char(args, '\"', 1);

    if(second_quote_pos == 0) {
        return false;
    }

    mstring_set_n(word, args, 1, second_quote_pos - 1);
    mstring_right(args, second_quote_pos + 1);
    mstring_strim(args, "  \r\n\t");
    return true;
}

bool cli_args_read_probably_quoted_string_and_trim(mstring_t* args, mstring_t* word) {
    if(mstring_size(args) > 1 && mstring_get_char(args, 0) == '\"') {
        return cli_args_read_quoted_string_and_trim(args, word);
    } else {
        return cli_args_read_string_and_trim(args, word);
    }
}