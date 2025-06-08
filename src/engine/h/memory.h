#ifndef MC_C_MEMORY_H
#define MC_C_MEMORY_H

#include <stdlib.h>
#include <string.h>

#include "logger.h"

#define MC_C_ALLOC(x, size)\
    if ((x) == NULL)\
    {\
        (x) = calloc(1, size);\
        if ((x) == NULL)\
        {\
            LOGFATAL("%s[%p] %s\n", #x, x, "Memory Allocation Failed, Process Aborted");\
            goto cleanup;\
        }\
        LOGDEBUG("%s[%p] %s\n", #x, x, "Memory Allocated");\
    }

#define MC_C_FREE(x, size)\
    if ((x) != NULL)\
    {\
        memset((x), 0, size);\
        free((x));\
        LOGDEBUG("%s[%p] %s\n", #x, x, "Unloaded");\
        (x) = NULL;\
    }

#define MC_C_CLEAR_MEM(x, size)\
    if ((x) != NULL)\
    {\
        memset((x), 0, size);\
        LOGDEBUG("%s[%p] %s\n", #x, x, "Cleared");\
    }

#endif /* MC_C_MEMORY_H */

