/**
 * @file helpers.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-11-24
 * 
 * Misc helpers
 */

#define COUNT_OF(x) (sizeof(x) / sizeof((x)[0]))
#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)