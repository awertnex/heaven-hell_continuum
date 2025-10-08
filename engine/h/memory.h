#ifndef ENGINE_MEMORY_H
#define ENGINE_MEMORY_H

#include "defines.h"
#include "limits.h"

#define arr_len(arr)    ((u64)sizeof(arr) / sizeof(arr[0]))

/* size = size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
b8 mem_alloc(void **x, u64 size, const str *name);

/* memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
b8 mem_alloc_memb(void **x, u64 memb, u64 size, const str *name);

/* memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
b8 mem_alloc_buf(buf *x, u64 memb, u64 size, const str *name);

/* size = size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
b8 mem_realloc(void **x, u64 size, const str *name);

/* memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
b8 mem_realloc_memb(void **x, u64 memb, u64 size, const str *name);

/* size = size in bytes,
 * name = pointer name (for logging) */
void mem_free(void **x, u64 size, const str *name);

/* name = pointer name (for logging) */
void mem_free_buf(buf *x, const str *name);

/* size = size in bytes,
 * name = pointer name (for logging) */
void mem_zero(void **x, u64 size, const str *name);

void print_bits(u64 x, u8 bit_count);
void swap_bits(char *c1, char *c2, u8 bit_count);
void swap_strings(str *s1, str *s2);

/* swap all occurrences of c1 in string with c2.
 *
 * return string */
str *swap_string_char(str *string, char c1, char c2);

/* return string format */
str *stringf(const str* format, ...);

void sort_buf(buf *s_buf);

#endif /* ENGINE_MEMORY_H */
