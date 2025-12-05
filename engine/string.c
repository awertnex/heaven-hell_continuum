#include <stdio.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/memory.h"
#include "h/string.h"

void swap_strings(str *s1, str *s2)
{
    u16 len = (strlen(s1) > strlen(s2)) ? strlen(s1) : strlen(s2);
    u16 i = 0;
    for (; i <= len; ++i)
        swap_bits(&s1[i], &s2[i]);
}

str *swap_string_char(str *string, char c1, char c2)
{
    u64 len = strlen(string);
    if (!len) return string;

    u64 i = 0;
    for (; i < len; ++i)
    {
        if (string[i] == c1)
            string[i] = c2;
    }

    return string;
}

str *stringf(const str* format, ...)
{
    static str str_buf[STRINGF_BUFFERS_MAX][OUT_STRING_MAX] = {0};
    static u64 index = 0;
    str *string = str_buf[index];

    __builtin_va_list args;
    __builtin_va_start(args, format);
    u64 required_bytes = vsnprintf(string, OUT_STRING_MAX, format, args);
    __builtin_va_end(args);

    if (required_bytes >= OUT_STRING_MAX - 1)
    {
        char *trunc_buf = str_buf[index] + OUT_STRING_MAX - 4;
        snprintf(trunc_buf, 4, "...");
    }

    index = (index + 1) % STRINGF_BUFFERS_MAX;
    return string;
}

u64 find_token(str *arg, int argc, char **argv)
{
    u32 i = 0;
    for (; (int)i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}
