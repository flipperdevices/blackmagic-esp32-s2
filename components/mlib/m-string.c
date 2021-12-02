#include "mlib/m-string.h"
#include "m-string.h"

struct mstring_t {
    string_t str;
};

mstring_t* mstring_alloc() {
    mstring_t* mstr = malloc(sizeof(mstring_t));
    string_init(mstr->str);
    return mstr;
}

mstring_t* mstring_alloc_set(const char* text) {
    mstring_t* mstr = malloc(sizeof(mstring_t));
    string_init(mstr->str);
    string_set_str(mstr->str, text);
    return mstr;
}

void mstring_free(mstring_t* mstr) {
    string_clear(mstr->str);
    free(mstr);
}

void mstring_set(mstring_t* mstr, const char* text) {
    string_set_str(mstr->str, text);
}

void mstring_reset(mstring_t* mstr) {
    string_reset(mstr->str);
}

void mstring_cat(mstring_t* mstr, const char* text) {
    string_cat_str(mstr->str, text);
}

int mstring_printf(mstring_t* mstr, const char format[], ...) {
    va_list args;
    va_start(args, format);
    int ret = string_vprintf(mstr->str, format, args);
    va_end(args);
    return ret;
}

int mstring_vprintf(mstring_t* mstr, const char format[], va_list args) {
    return string_vprintf(mstr->str, format, args);
}

const char* mstring_get_cstr(const mstring_t* mstr) {
    return string_get_cstr(mstr->str);
}

size_t mstring_size(const mstring_t* mstr) {
    return string_size(mstr->str);
}

char mstring_get_char(const mstring_t* mstr, size_t index) {
    return string_get_char(mstr->str, index);
}

int mstring_cmp_cstr(const mstring_t* mstr, const char* cstr) {
    return string_cmp_str(mstr->str, cstr);
}

void mstring_strim(mstring_t* mstr, const char charac[]) {
    string_strim(mstr->str, charac);
}

size_t mstring_search_char(mstring_t* mstr, char c, size_t start) {
    return string_search_char(mstr->str, c, start);
}

void mstring_set_n(mstring_t* mstr, const mstring_t* mstr_ref, size_t offset, size_t length) {
    string_set_n(mstr->str, mstr_ref->str, offset, length);
}

void mstring_set_strn(mstring_t* mstr, const char str[], size_t n) {
    string_set_strn(mstr->str, str, n);
}

void mstring_push_back(mstring_t* mstr, char c) {
    string_push_back(mstr->str, c);
}

void mstring_right(mstring_t* mstr, size_t index) {
    string_right(mstr->str, index);
}

size_t mstring_count_char(mstring_t* mstr, char c) {
    const char* str = string_get_cstr(mstr->str);
    size_t count = 0;

    for(size_t i = 0; i < string_size(mstr->str); i++) {
        if(str[i] == c) count++;
    }

    return count;
}