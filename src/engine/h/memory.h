#ifndef MC_C_MEMORY_H
#define MC_C_MEMORY_H

#include <stdlib.h>
#include <string.h>

#include "logger.h"

#define MC_C_ALLOC(x, size)\
    if ((x) == NULL)\
    {\
        (x) = malloc(size);\
        if ((x) == NULL)\
        {\
            LOGFATAL("%s[%p] Memory Allocation Failed, Process Aborted", #x, &(x));\
            goto cleanup;\
        }\
        memset((x), 0, (size));\
        LOGINFO("%s Memory Allocated", #x);\
    }

#define MC_C_FREE(x, size)\
    if ((x))\
    {\
        memset((x), 0, size);\
        free((x));\
        (x) = NULL;\
        LOGINFO("%s[%p] Unloaded", #x, x);\
    }

#define MC_C_CLEAR_MEM(x, size)\
    if ((x) != NULL)\
    {\
        memset((x), 0, size);\
        LOGINFO("%s[%p] Cleared", #x, x);\
    }

#endif /* MC_C_MEMORY_H */

