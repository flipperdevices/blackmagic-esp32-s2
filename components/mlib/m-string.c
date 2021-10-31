#include "mlib\m-string.h"
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

void mstring_cat(mstring_t* mstr, const char* text) 
{
    string_cat_str(mstr->str, text);
}

int mstring_printf(mstring_t* mstr, const char format[], ...) {
    va_list args;
    va_start(args, format);
    int ret = string_vprintf(mstr->str, format, args);
    va_end(args);
    return ret;
}

const char* mstring_get_cstr(const mstring_t* mstr) {
    return string_get_cstr(mstr->str);
}

char mstring_get_char(const mstring_t* mstr, size_t index) {
    return string_get_char(mstr->str, index);
}