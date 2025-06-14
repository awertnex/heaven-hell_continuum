#ifndef ENGINE_MEMORY_H
#define ENGINE_MEMORY_H

#include "defines.h"

#define arr_len(arr)    ((u64)sizeof(arr) / sizeof(arr[0]))

/* 
 * size = size in bytes;
 * name = pointer name (for logging);
 * returns alloc status as boolean;
 */
b8 mem_alloc(void **x, u64 size, const str *name);

/* 
 * memb = number of members;
 * size = member size in bytes;
 * name = pointer name (for logging);
 * returns alloc status as boolean;
 */
b8 mem_alloc_memb(void **x, u64 memb, u64 size, const str *name);

/* 
 * memb = number of members;
 * size = member size in bytes;
 * name = pointer name (for logging);
 * returns alloc status as boolean;
 */
b8 mem_alloc_str_buf(str_buf *x, u64 memb, u64 size, const str *name);

/* 
 * size = size in bytes;
 * name = pointer name (for logging);
 * returns realloc status as boolean;
 */
b8 mem_realloc(void **x, u64 size, const str *name);

/* 
 * memb = number of members;
 * size = member size in bytes;
 * name = pointer name (for logging);
 * returns realloc status as boolean;
 */
b8 mem_realloc_memb(void **x, u64 memb, u64 size, const str *name);

/* 
 * size = size in bytes;
 * name = pointer name (for logging);
 */
void mem_free(void **x, u64 size, const str *name);

/* 
 * memb_size = member size in bytes (member count is taken care of);
 * name = pointer name (for logging);
 */
void mem_free_str_buf(str_buf *x, u64 memb_size, const str *name);

/* 
 * size = size in bytes;
 * name = pointer name (for logging);
 */
void mem_zero(void **x, u64 size, const str *name);

void print_bits(u64 x, u8 bit_count);
void swap_bits(char *c1, char *c2, u8 bit_count);
void swap_strings(str *s1, str *s2);
void sort_str_buf(str_buf *s_buf);

#endif /* ENGINE_MEMORY_H */

