/**
 * @file m-string.h
 * 
 * M*Lib string wrapper, to prevent inline code pollution
 * 
 */
#pragma once

typedef struct mstring_t mstring_t;

/**
 * Allocate string
 * @return mstring_t* 
 */
mstring_t* mstring_alloc();

/**
 * Allocate string and set value
 * @param text 
 * @return mstring_t* 
 */
mstring_t* mstring_alloc_set(const char* text);

/**
 * Free string
 * @param mstr 
 */
void mstring_free(mstring_t* mstr);

/**
 * Set string value
 * @param mstr 
 * @param text 
 */
void mstring_set(mstring_t* mstr, const char* text);

/**
 * Reset string value
 * @param mstr 
 * @param text 
 */
void mstring_reset(mstring_t* mstr);

/**
 * Concatenate value
 * @param mstr 
 * @param text 
 */
void mstring_cat(mstring_t* mstr, const char* text);

/**
 * Printf string value
 * @param mstr 
 * @param format 
 * @param ... 
 * @return int 
 */
int mstring_printf(mstring_t* mstr, const char format[], ...);

/**
 * Get constant pointer to string value
 * @param mstr 
 * @return const char* 
 */
const char* mstring_get_cstr(const mstring_t* mstr);

/**
 * Get char at index
 * @param mstr 
 * @param index 
 * @return char 
 */
char mstring_get_char(const mstring_t* mstr, size_t index);