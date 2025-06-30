#ifndef ENGINE_LIMITS_H
#define ENGINE_LIMITS_H

#if defined(__linux__) || defined(__linux)
    #include <linux/limits.h>
#elif defined(__WIN32) || defined(__WIN64) || defined (__CYGWIN__)
    #include <limits.h>
    #define NAME_MAX 255
#endif /* PLATFORM */

#define GLYPH_MAX   256

#endif /* ENGINE_LIMITS_H */

