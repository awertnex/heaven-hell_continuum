#ifndef ENGINE_STRING_H
#define ENGINE_STRING_H

#include "types.h"

void swap_strings(str *s1, str *s2);

/* swap all occurrences of 'c1' in 'string' with 'c2'.
 *
 * return 'string' */
str *swap_string_char(str *string, char c1, char c2);

/* return static formatted string.
 *
 * inspired by Raylib: 'github.com/raysan5/raylib':
 * raylib/src/rtext.c/TextFormat() */
str *stringf(const str* format, ...);

/* compare 'arg' to any of 'argv' entries.
 *
 * return 'argc' of match if match found,
 * return 0 if no matches found */
u64 find_token(str *arg, int argc, str **argv);

/* load tokens from file at 'path' into a 'KeyValue' buffer as
 * 'str' and 'u64' respectively.
 *
 * return 'KeyValue' buffer of tokens,
 * return (KeyValue){0} on failure and 'engine_err' is set accordingly */
KeyValue get_tokens_key_val(const str *path);

#endif /* ENGINE_STRING_H */
