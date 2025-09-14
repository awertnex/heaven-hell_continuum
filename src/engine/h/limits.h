#ifndef ENGINE_LIMITS_H
#define ENGINE_LIMITS_H

#if defined(__linux__) || defined(__linux)
    #include <linux/limits.h>
#elif defined(__WIN32) || defined(__WIN64) || defined (__CYGWIN__)
    #define NAME_MAX 255
#endif /* PLATFORM */

#define U8_MAX 255
#define I8_MAX 127
#define I8_MIN (-I8_MAX - 1)

#define U16_MAX 65535
#define I16_MAX 32767
#define I16_MIN (-I8_MAX - 1)

#define U32_MAX 4294967295
#define I32_MAX 2147483647
#define I32_MIN (-I8_MAX - 1)

#define U64_MAX 18446744073709551615
#define I64_MAX 9223372036854775807
#define I64_MIN (-I8_MAX - 1)

#define GLYPH_MAX   256

#endif /* ENGINE_LIMITS_H */

