#ifndef MC_C_MEMORY_H
#define MC_C_MEMORY_H

#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "logger.h"

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
    LOGDEBUG("%s[%p] %s\n", name, (void *)(uintptr_t)(*x), "Memory Allocated");

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
    LOGDEBUG("%s[%p] %s\n", name, (void *)(uintptr_t)(*x), "Unloaded");
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
    LOGDEBUG("%s[%p] %s\n", name, (void *)(uintptr_t)(*x), "Cleared");
}

#endif /* MC_C_MEMORY_H */

