#ifndef MC_C_MEMORY_H
#define MC_C_MEMORY_H

#include <stdlib.h>
#include <string.h>

#include "logger.h"

#define MC_C_ALLOC(x, len)                                                      \
    if ((x) == NULL)                                                            \
    {                                                                           \
        (x) = malloc(len);                                                      \
        if ((x) == NULL)                                                        \
        {                                                                       \
            LOGFATAL("%s %s", #x, "Memory Allocation Failed, Process Aborted"); \
            goto cleanup;                                                       \
        }                                                                       \
        memset((x), 0, (len));                                                  \
        LOGINFO("%s %s", #x, "Memory Allocated");                               \
    }

#define MC_C_FREE(x, len)                                                       \
    if ((x))                                                                    \
    {                                                                           \
        memset((x), 0, len);                                                    \
        free((x));                                                              \
        (x) = NULL;                                                             \
        LOGINFO("%s Unloaded", #x);                                             \
    }

#define MC_C_CLEAR_MEM(x, len)                                                  \
    if ((x))                                                                    \
    {                                                                           \
        memset((x), 0, len);                                                    \
        LOGINFO("%s Cleared", #x);                                              \
    }                                                                           \

#endif /* MC_C_MEMORY_H */

