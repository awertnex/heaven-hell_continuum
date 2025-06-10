#ifndef MC_C_ENGINE_MEMORY_H
#define MC_C_ENGINE_MEMORY_H

#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "logger.h"

#define arr_len(arr)    ((u64)sizeof(arr) / sizeof(arr[0]))

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
        LOGFATAL("%s[%p] %s\n", name, (void *)(uintptr_t)(*x), "Memory Allocation Failed, Process Aborted");
        return FALSE;
    }
    LOGDEBUG("%s[%p] %s[%lldB]\n", name, (void *)(uintptr_t)(*x), "Memory Allocated", size);

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
        LOGFATAL("%s[%p] %s\n", name, (void *)(uintptr_t)(*x), "Memory Allocation Failed, Process Aborted");
        return FALSE;
    }
    LOGDEBUG("%s[%p] %s[%lldB]\n", name, (void *)(uintptr_t)(*x), "Memory Allocated", memb * size);

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
    LOGDEBUG("%s[%p] %s[%lldB]\n", name, (void *)(uintptr_t)(*x), "Unloaded", size);
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
    LOGDEBUG("%s[%p] %s[%lldB]\n", name, (void *)(uintptr_t)(*x), "Memory Cleared", size);
}

#endif /* MC_C_ENGINE_MEMORY_H */

