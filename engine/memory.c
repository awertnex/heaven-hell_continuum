#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "h/platform.h"
#include "h/memory.h"
#include "h/limits.h"
#include "h/logger.h"

b8
mem_alloc(void **x, u64 size, const str *name)
{
    if (*x != NULL)
        return TRUE;

    *x = calloc(1, size);
    if (*x == NULL)
    {
        LOGFATAL("%s[%p] Memory Allocation Failed, Process Aborted\n",
                name, (void*)(uintptr_t)(*x));
        return FALSE;
    }
    LOGTRACE("%s[%p] Memory Allocated[%lldB]\n",
            name, (void*)(uintptr_t)(*x), size);

    return TRUE;
}

b8
mem_alloc_memb(void **x, u64 memb, u64 size, const str *name)
{
    if (*x != NULL)
        return TRUE;

    *x = calloc(memb, size);
    if (*x == NULL)
    {
        LOGFATAL("%s[%p] Memory Allocation Failed, Process Aborted\n",
                name, (void*)(uintptr_t)(*x));
        return FALSE;
    }
    LOGTRACE("%s[%p] Memory Allocated[%lldB]\n",
            name, (void*)(uintptr_t)(*x), memb * size);

    return TRUE;
}

b8
mem_alloc_buf(buf *x, u64 memb, u64 size, const str *name)
{
    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    snprintf(name_i, NAME_MAX, "%s.i", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (!mem_alloc_memb((void*)&x->i, memb, sizeof(str*), name_i))
        return FALSE;

    if (!mem_alloc_memb((void*)&x->buf, memb, size, name_buf))
    {
        mem_free((void*)&x->i, memb * sizeof(str*), name_i);
        return FALSE;
    }

    for (u64 i = 0; i < memb; ++i)
        x->i[i] = x->buf + (i * size);

    x->memb = memb;
    x->size = size;
    x->loaded = TRUE;
    return TRUE;
}

b8
mem_realloc(void **x, u64 size, const str *name)
{
    if (*x == NULL)
    {
        LOGERROR("%s[%p] Memory Reallocation Failed, Pointer NULL\n",
                name, (void*)(uintptr_t)(*x));
        return FALSE;
    }

    void *temp = realloc(*x, size);
    if (temp == NULL)
    {
        LOGFATAL("%s[%p] Memory Reallocation Failed, Process Aborted\n",
                name, (void*)(uintptr_t)(*x));
        return FALSE;
    }

    *x = temp;
    LOGTRACE("%s[%p] Memory Reallocated[%lldB]\n",
            name, (void*)(uintptr_t)(*x), size);

    return TRUE;
}

b8
mem_realloc_memb(void **x, u64 memb, u64 size, const str *name)
{
    if (*x == NULL)
    {
        LOGERROR("%s[%p] Memory Reallocation Failed, Pointer NULL\n",
                name, (void*)(uintptr_t)(*x));
        return FALSE;
    }

    void *temp = realloc(*x, memb * size);
    if (temp == NULL)
    {
        LOGFATAL("%s[%p] Memory Reallocation Failed, Process Aborted\n",
                name, (void*)(uintptr_t)(*x));
        return FALSE;
    }

    *x = temp;
    LOGTRACE("%s[%p] Memory Reallocated[%lldB]\n",
            name, (void*)(uintptr_t)(*x), memb * size);

    return TRUE;
}

void
mem_free(void **x, u64 size, const str *name)
{
    if (*x == NULL)
        return;

    memset(*x, 0, size);
    free(*x);
    LOGTRACE("%s[%p] Memory Unloaded[%lldB]\n",
            name, (void*)(uintptr_t)(*x), size);
    *x = NULL;
}

void
mem_free_buf(buf *x, const str *name)
{
    if (x->i != NULL)
    {
        memset(x->i, 0, x->memb * sizeof(str*));
        free(x->i);
        LOGTRACE("%s.i[%p] Memory Unloaded[%lldB]\n",
                name, (void*)(uintptr_t)(x->i), x->memb * sizeof(str*));
    }

    if (x->buf != NULL)
    {
        memset(x->buf, 0, x->memb * x->size);
        free(x->buf);
        LOGTRACE("%s.buf[%p] Memory Unloaded[%lldB]\n",
                name, (void*)(uintptr_t)(x->buf), x->memb * x->size);
    }

    *x = (buf){NULL};
}

void
mem_zero(void **x, u64 size, const str *name)
{
    if (*x == NULL)
        return;

    memset(*x, 0, size);
    LOGTRACE("%s[%p] Memory Cleared[%lldB]\n",
            name, (void*)(uintptr_t)(*x), size);
}

void
print_bits(u64 x, u8 bit_count)
{
    while(bit_count--)
        putchar('0' + ((x >> bit_count) & 1));
    putchar('\n');
}

void
swap_bits(char *c1, char *c2, u8 bit_count)
{
    for (u8 i = 0; i < bit_count; ++i)
    {
        if (((*c1 >> i) & 1) == ((*c2 >> i) & 1))
            continue;

        *c1 ^= (1 << i);
        *c2 ^= (1 << i);
    }
}

void
swap_strings(str *s1, str *s2)
{
    u16 len = (strlen(s1) > strlen(s2)) ? strlen(s1) : strlen(s2);
    for (u16 i = 0; i <= len; ++i)
        swap_bits(&s1[i], &s2[i], 8);
}

str *
swap_string_char(str *string, char c1, char c2)
{
    u64 len = strlen(string);
    if (!len) return string;

    for (u64 i = 0; i < len; ++i)
    {
        if (string[i] == c1)
            string[i] = c2;
    }

    return string;
}

/* inspired by 'github.com/raysan5/raylib':
 * raylib/src/rtext.c/TextFormat() */
str *
stringf(const str* format, ...)
{
    static str str_buf[STRINGF_BUFFERS_MAX][OUT_STRING_MAX] = {0};
    static u64 index = 0;

    str *string = str_buf[index];
    mem_zero((void*)&string, OUT_STRING_MAX, "stringf_current_buf");

    __builtin_va_list args;
    va_start(args, format);
    u64 required_bytes = vsnprintf(string, OUT_STRING_MAX, format, args);
    va_end(args);

    if (required_bytes >= OUT_STRING_MAX - 1)
    {
        char *trunc_buf = str_buf[index] + OUT_STRING_MAX - 4;
        snprintf(trunc_buf, 4, "...");
    }

    index = ++index % STRINGF_BUFFERS_MAX;
    return string;
}

void
sort_buf(buf *buffer) /* TODO: fucking fix this */
{
#if 0
    for (u16 i = 0, smallest = 0; i < buffer->memb - 1 && buffer->i[i] != NULL; ++i)
    {
        smallest = i;
        for (u64 j = i + 1; j < buffer->memb && buffer->i[j] != NULL; ++j)
        {
            char cmp0 = tolower(buffer->i[j] + 0);
            char cmp1 = tolower(buffer->i[smallest] + 0);
            if (cmp0 < cmp1 && buffer->i[j][0] && buffer->i[smallest][0])
                smallest = j;

            if (cmp0 == cmp1 && buffer->i[j] && cmp1)
                for (u16 k = 1; k < NAME_MAX - 1 && buffer->i[j][k - 1] && buffer->i[smallest][k - 1]; ++k)
                {
                    if (tolower(buffer->i[j] + k) < tolower(buffer->i[smallest] + k))
                    {
                        smallest = j;
                        break;
                    }
                }
        }

        swap_strings(buffer->i[i], buffer->i[smallest]);
    }
#endif
}
