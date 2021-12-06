/**
 * @file m-string.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-11-21
 * 
 * M*Lib string wrapper, to prevent inline code pollution
 */
#pragma once
#include <stdlib.h>
#include <stdarg.h>

typedef struct mstring_t mstring_t;

#define STRING_FAILURE ((size_t)-1)

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
 * 
 * @param mstr 
 * @param format 
 * @param args 
 * @return int 
 */
int mstring_vprintf(mstring_t* mstr, const char format[], va_list args);

/**
 * Get constant pointer to string value
 * @param mstr 
 * @return const char* 
 */
const char* mstring_get_cstr(const mstring_t* mstr);

/**
 * Get constant pointer to string value
 * @param mstr 
 * @return size_t 
 */
size_t mstring_size(const mstring_t* mstr);

/**
 * Get char at index
 * @param mstr 
 * @param index 
 * @return char 
 */
char mstring_get_char(const mstring_t* mstr, size_t index);

/**
 * Compare the string to the other string
 * @param mstr 
 * @param mstr2 
 * @return int 
 */
int mstring_cmp(const mstring_t* mstr, const mstring_t* mstr2);

/**
 * Compare the string to the C string
 * @param mstr 
 * @param cstr 
 * @return int 
 */
int mstring_cmp_cstr(const mstring_t* mstr, const char* cstr);

/**
 * Remove any characters from c_array that are present in the begining of the string and the end of the string
 * @param mstr 
 * @param charac 
 */
void mstring_strim(mstring_t* mstr, const char c_array[]);

/**
 * Search for the position of the character c
 * from the position 'start' (include)  in the string 
 * Return STRING_FAILURE if not found.
 * @param mstr 
 * @param c 
 * @param start 
 * @return size_t 
 */
size_t mstring_search_char(mstring_t* mstr, char c, size_t start);

/**
 * Set the string to the n first characters of other one
 * @param mstr 
 * @param mstr_ref 
 * @param offset 
 * @param length 
 */
void mstring_set_n(mstring_t* mstr, const mstring_t* mstr_ref, size_t offset, size_t length);

void mstring_set_strn(mstring_t* mstr, const char str[], size_t n);

void mstring_push_back(mstring_t* mstr, char c);

void mstring_right(mstring_t* mstr, size_t index);

size_t mstring_count_char(mstring_t* mstr, char c);