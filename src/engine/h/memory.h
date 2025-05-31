#ifndef MC_C_MEMORY_H

#include <stdlib.h>
#include <string.h>

#include "logger.h"

#define MC_C_ALLOC(x, size)                                                     \
    if ((x) == NULL)                                                            \
    {                                                                           \
        (x) = malloc(size);                                                     \
        if ((x) == NULL)                                                        \
        {                                                                       \
            LOGFATAL("%s %s", #x, "Memory Allocation Failed, Process Aborted"); \
            goto cleanup;                                                       \
        }                                                                       \
        memset((x), 0, (size));                                                 \
        LOGINFO("%s %s", #x, "Memory Allocated");                               \
    }

#define MC_C_FREE(x, size)                                                      \
    if ((x))                                                                    \
    {                                                                           \
        memset((x), 0, (size));                                                 \
        free((x));                                                              \
        (x) = NULL;                                                             \
        LOGINFO("%s Unloaded", #x);                                             \
    }

#define MC_C_CLEAR_MEM(x, size)                                                 \
    if ((x))                                                                    \
    {                                                                           \
        memset((x), 0, (size));                                                 \
        LOGINFO("%s Cleared", #x);                                              \
    }                                                                           \

#define MC_C_MEMORY_H
#endif

