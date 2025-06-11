#ifndef MC_C_ENGINE_MEMORY_H
#define MC_C_ENGINE_MEMORY_H

#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "logger.h"

#define arr_len(arr)    ((u64)sizeof(arr) / sizeof(arr[0]))

typedef struct str_buf
{
    str **entry;
    u64 count;
    str *buf;
} str_buf;
/* 
 * size = size in bytes;
 * name = pointer name (for logging);
 */
static inline b8 mem_alloc(void **x, u64 size, const str *name)
{
    if (*x != NULL) return TRUE;

    *x = calloc(1, size);
    if (*x == NULL)
    {
        LOGFATAL("%s[%p] %s\n", name, (void*)(uintptr_t)(*x), "Memory Allocation Failed, Process Aborted");
        return FALSE;
    }
    LOGTRACE("%s[%p] %s[%lldB]\n", name, (void*)(uintptr_t)(*x), "Memory Allocated", size);

    return TRUE;
}

/* 
 * memb = number of members;
 * size = member size in bytes;
 * name = pointer name (for logging);
 */
static inline b8 mem_alloc_memb(void **x, u64 memb, u64 size, const str *name)
{
    if (*x != NULL) return TRUE;

    *x = calloc(memb, size);
    if (*x == NULL)
    {
        LOGFATAL("%s[%p] %s\n", name, (void*)(uintptr_t)(*x), "Memory Allocation Failed, Process Aborted");
        return FALSE;
    }
    LOGTRACE("%s[%p] %s[%lldB]\n", name, (void*)(uintptr_t)(*x), "Memory Allocated", memb * size);

    return TRUE;
}

/* 
 * size = size in bytes;
 * name = pointer name (for logging);
 */
static inline void mem_free(void **x, u64 size, const str *name)
{
    if (*x == NULL) return;

    memset(*x, 0, size);
    free(*x);
    LOGTRACE("%s[%p] %s[%lldB]\n", name, (void*)(uintptr_t)(*x), "Unloaded", size);
    *x = NULL;
}

/* 
 * size = size in bytes;
 * name = pointer name (for logging);
 */
static inline void mem_zero(void **x, u64 size, const str *name)
{
    if (*x == NULL) return;

    memset(*x, 0, size);
    LOGTRACE("%s[%p] %s[%lldB]\n", name, (void*)(uintptr_t)(*x), "Memory Cleared", size);
}

#endif /* MC_C_ENGINE_MEMORY_H */

