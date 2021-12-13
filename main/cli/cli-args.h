/**
 * @file cli-args.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-01
 * 
 * 
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <m-string.h>

/** Extract int value and trim arguments string
 * @param args - arguments string 
 * @param word first argument, output
 * @return true - success
 * @return false - arguments string does not contain int
 */
bool cli_args_read_int_and_trim(mstring_t* args, int* value);

/**
 * @brief Extract first argument from arguments string and trim arguments string
 * @param args arguments string
 * @param word first argument, output
 * @return true - success
 * @return false - arguments string does not contain anything
 */
bool cli_args_read_string_and_trim(mstring_t* args, mstring_t* word);

/**
 * @brief Extract the first quoted argument from the argument string and trim the argument string. If the argument is not quoted, returns false.
 * @param args arguments string
 * @param word first argument, output, without quotes
 * @return true - success
 * @return false - arguments string does not contain anything or not quoted
 */
bool cli_args_read_quoted_string_and_trim(mstring_t* args, mstring_t* word);

/**
 * @brief Extract the first quoted argument from the argument string and trim the argument string. If the argument is not quoted, calls args_read_string_and_trim.
 * @param args arguments string
 * @param word first argument, output, without quotes
 * @return true - success
 * @return false - arguments string does not contain anything
 */
bool cli_args_read_probably_quoted_string_and_trim(mstring_t* args, mstring_t* word);

/************************************ HELPERS ***************************************/

/**
 * @brief Get length of first word from arguments string
 * @param args arguments string
 * @return size_t length of first word
 */
size_t cli_args_get_first_word_length(mstring_t* args);

/**
 * @brief Get length of arguments string
 * @param args arguments string
 * @return size_t length of arguments string
 */
size_t cli_args_length(mstring_t* args);