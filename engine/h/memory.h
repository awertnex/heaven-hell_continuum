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

#define mem_map(x, size, name) \
    _mem_map(x, size, name, __FILE__, __LINE__)

#define mem_commit(x, offset, size, name) \
    _mem_commit(x, offset, size, name, __FILE__, __LINE__)

#define mem_unmap(x, size, name) \
    _mem_unmap(x, size, name, __FILE__, __LINE__)

/* -- INTERNAL USE ONLY --;
 *
 * size = size in bytes,
 * name = pointer name (for logging).
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 _mem_alloc(void **x, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 _mem_alloc_memb(void **x, u64 memb, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 _mem_alloc_buf(buf *x, u64 memb, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * size = size in bytes,
 * name = pointer name (for logging).
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 _mem_realloc(void **x, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * memb = number of members,
 * size = member size in bytes,
 * name = pointer name (for logging).
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 _mem_realloc_memb(void **x, u64 memb, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * size = size in bytes,
 * name = pointer name (for logging) */
void _mem_free(void **x, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * name = pointer name (for logging) */
void _mem_free_buf(buf *x,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * size = size in bytes,
 * name = pointer name (for logging).
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 _mem_zero(void **x, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 * size = size in bytes,
 * name = pointer name (for logging).
 *
 * reserve a block of memory for x.
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 _mem_map(void **x, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 * size = size in bytes,
 * name = pointer name (for logging).
 *
 * commit a block of mapped memory for x.
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 _mem_commit(void **x, void *offset, u64 size,
        const str *name, const str *file, u64 line);

/* -- INTERNAL USE ONLY --;
 *
 * -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 * size = size in bytes,
 * name = pointer name (for logging).
 *
 * unmap a block of memory x */
void _mem_unmap(void **x, u64 size,
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

/* return static formatted string.
 *
 * inspired by Raylib: 'github.com/raysan5/raylib':
 * raylib/src/rtext.c/TextFormat() */
str *stringf(const str* format, ...);

void sort_buf(buf *s_buf);

#endif /* ENGINE_MEMORY_H */
