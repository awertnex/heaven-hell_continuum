#ifndef ENGINE_MEMORY_H
#define ENGINE_MEMORY_H

#include "defines.h"
#include "limits.h"

#define arr_len(arr)    ((u64)sizeof(arr) / sizeof(arr[0]))

#define mem_alloc(x, size, name) \
    _mem_alloc(x, size, name, __FILE__, __LINE__)

#define mem_alloc_memb(x, memb, size, name) \
    _mem_alloc_memb(x, memb, size, name, __FILE__, __LINE__)

#define mem_alloc_buf(x, memb, size, name) \
    _mem_alloc_buf(x, memb, size, name, __FILE__, __LINE__)

#define mem_realloc(x, size, name) \
    _mem_realloc(x, size, name, __FILE__, __LINE__)

#define mem_realloc_memb(x, memb, size, name) \
    _mem_realloc_memb(x, memb, size, name, __FILE__, __LINE__)

#define mem_free(x, size, name) \
    _mem_free(x, size, name, __FILE__, __LINE__)

#define mem_free_buf(x, name) \
    _mem_free_buf(x, name, __FILE__, __LINE__)

#define mem_zero(x, size, name) \
    _mem_zero(x, size, name, __FILE__, __LINE__)

/* size = size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
extern b8 _mem_alloc(void **x, u64 size, const str *name,
        const str *file, u64 line);

/* memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
extern b8 _mem_alloc_memb(void **x, u64 memb, u64 size,
        const str *name, const str *file, u64 line);

/* memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
extern b8 _mem_alloc_buf(buf *x, u64 memb, u64 size,
        const str *name, const str *file, u64 line);

/* size = size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
extern b8 _mem_realloc(void **x, u64 size,
        const str *name, const str *file, u64 line);

/* memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return FALSE (0) on failure */
extern b8 _mem_realloc_memb(void **x, u64 memb, u64 size,
        const str *name, const str *file, u64 line);

/* size = size in bytes,
 * name = pointer name (for logging) */
extern void _mem_free(void **x, u64 size,
        const str *name, const str *file, u64 line);

/* name = pointer name (for logging) */
extern void _mem_free_buf(buf *x,
        const str *name, const str *file, u64 line);

/* size = size in bytes,
 * name = pointer name (for logging) */
extern void _mem_zero(void **x, u64 size,
        const str *name, const str *file, u64 line);

void print_bits(u64 x, u8 bit_count);
void swap_bits(char *c1, char *c2);
void swap_bits_u8(u8 *a, u8 *b);
void swap_bits_u32(u32 *a, u32 *b);
void swap_bits_u64(u64 *a, u64 *b);
void swap_strings(str *s1, str *s2);

/* swap all occurrences of c1 in string with c2.
 *
 * return string */
str *swap_string_char(str *string, char c1, char c2);

/* return formatted string.
 *
 * inspired by 'github.com/raysan5/raylib':
 * raylib/src/rtext.c/TextFormat() */
str *stringf(const str* format, ...);

void sort_buf(buf *s_buf);

#endif /* ENGINE_MEMORY_H */
