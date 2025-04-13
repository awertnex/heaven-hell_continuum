#pragma once

#include <stdlib.h>

#include "logger.h"

#define MC_C_ALLOC(ptr, size)                                                   \
    (ptr) = malloc(size);                                                       \
    if ((ptr) == NULL) {                                                        \
        LOGFATAL("%s %s", #ptr, "Memory Allocation Failed, Aborting Process");  \
        goto cleanup;                                                           \
    }                                                                           \
    memset((ptr), 0, (size));

#define MC_C_FREE(ptr, size)                                                    \
    if ((ptr)) {                                                                \
        memset((ptr), 0, (size));                                               \
        free((ptr));                                                            \
        (ptr) = NULL;                                                           \
        LOGINFO("%s Unloaded", #ptr);                                           \
    }

