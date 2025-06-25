#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "h/limits.h"

#include "src/engine/h/platform.h"
#include "h/memory.h"
#include "h/logger.h"

b8 mem_alloc(void **x, u64 size, const str *name)
{
    if (*x != NULL)
        return TRUE;

    *x = calloc(1, size);
    if (*x == NULL)
    {
        LOGFATAL("%s[%p] Memory Allocation Failed, Process Aborted\n", name, (void*)(uintptr_t)(*x));
        return FALSE;
    }
    LOGTRACE("%s[%p] Memory Allocated[%lldB]\n", name, (void*)(uintptr_t)(*x), size);

    return TRUE;
}

b8 mem_alloc_memb(void **x, u64 memb, u64 size, const str *name)
{
    if (*x != NULL)
        return TRUE;

    *x = calloc(memb, size);
    if (*x == NULL)
    {
        LOGFATAL("%s[%p] Memory Allocation Failed, Process Aborted\n", name, (void*)(uintptr_t)(*x));
        return FALSE;
    }
    LOGTRACE("%s[%p] Memory Allocated[%lldB]\n", name, (void*)(uintptr_t)(*x), memb * size);

    return TRUE;
}

b8 mem_alloc_str_buf(str_buf *x, u64 memb, u64 size, const str *name)
{
    str name_entry[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    snprintf(name_entry, NAME_MAX, "%s.entry", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (!mem_alloc_memb((void*)&x->entry, memb, sizeof(str*), name_entry))
        return FALSE;

    if (!mem_alloc_memb((void*)&x->buf, memb, size, name_buf))
    {
        mem_free((void*)&x->entry, memb * sizeof(str*), name_entry);
        return FALSE;
    }

    for (u64 i = 0; i < memb; ++i)
        x->entry[i] = x->buf + (i * size);

    x->count = memb;
    x->loaded = TRUE;
    return TRUE;
}

b8 mem_realloc(void **x, u64 size, const str *name)
{
    if (*x == NULL)
    {
        LOGERROR("%s[%p] Memory Reallocation Denied, Pointer NULL\n", name, (void*)(uintptr_t)(*x));
        return FALSE;
    }

    void *temp = realloc(*x, size);
    if (temp == NULL)
    {
        LOGFATAL("%s[%p] Memory Reallocation Failed, Process Aborted\n", name, (void*)(uintptr_t)(*x));
        return FALSE;
    }

    *x = temp;
    LOGTRACE("%s[%p] Memory Reallocated[%lldB]\n", name, (void*)(uintptr_t)(*x), size);

    return TRUE;
}

b8 mem_realloc_memb(void **x, u64 memb, u64 size, const str *name)
{
    if (*x == NULL)
    {
        LOGERROR("%s[%p] Memory Reallocation Denied, Pointer NULL\n", name, (void*)(uintptr_t)(*x));
        return FALSE;
    }

    void *temp = realloc(*x, memb * size);
    if (temp == NULL)
    {
        LOGFATAL("%s[%p] Memory Reallocation Failed, Process Aborted\n", name, (void*)(uintptr_t)(*x));
        return FALSE;
    }

    *x = temp;
    LOGTRACE("%s[%p] Memory Reallocated[%lldB]\n", name, (void*)(uintptr_t)(*x), memb * size);

    return TRUE;
}

void mem_free(void **x, u64 size, const str *name)
{
    if (*x == NULL)
        return;

    memset(*x, 0, size);
    free(*x);
    LOGTRACE("%s[%p] Memory Unloaded[%lldB]\n", name, (void*)(uintptr_t)(*x), size);
    *x = NULL;
}

void mem_free_str_buf(str_buf *x, u64 memb_size, const str *name)
{
    if (x->entry != NULL)
    {
        memset(x->entry, 0, x->count * sizeof(str*));
        free(x->entry);
        LOGTRACE("%s.entry[%p] Memory Unloaded[%lldB]\n", name, (void*)(uintptr_t)(x->entry), x->count * sizeof(str*));
        x->entry = NULL;
    }

    if (x->buf != NULL)
    {
        memset(x->buf, 0, x->count * memb_size);
        free(x->buf);
        LOGTRACE("%s.buf[%p] Memory Unloaded[%lldB]\n", name, (void*)(uintptr_t)(x->buf), x->count * memb_size);
        x->buf = NULL;
    }

    x->count = 0;
    x->loaded = FALSE;
}

void mem_zero(void **x, u64 size, const str *name)
{
    if (*x == NULL)
        return;

    memset(*x, 0, size);
    LOGTRACE("%s[%p] Memory Cleared[%lldB]\n", name, (void*)(uintptr_t)(*x), size);
}

void print_bits(u64 x, u8 bit_count)
{
    while(bit_count--)
        putchar('0' + ((x >> bit_count) & 1));
    putchar('\n');
}

void swap_bits(char *c1, char *c2, u8 bit_count)
{
    for (u8 i = 0; i < bit_count; ++i)
    {
        if (((*c1 >> i) & 1) == ((*c2 >> i) & 1))
            continue;

        *c1 ^= (1 << i);
        *c2 ^= (1 << i);
    }
}

void swap_strings(str *s1, str *s2)
{
    u16 len = (strlen(s1) > strlen(s2)) ? strlen(s1) : strlen(s2);
    for (u16 i = 0; i <= len; ++i)
        swap_bits(&s1[i], &s2[i], 8);
}

void sort_str_buf(str_buf *s_buf)
{
    for (u16 i = 0, smallest = 0; i < s_buf->count - 1 && s_buf->entry[i] != NULL; ++i)
    {
        smallest = i;
        for (u64 j = i + 1; j < s_buf->count && s_buf->entry[j] != NULL; ++j)
        {
            if (tolower(s_buf->entry[j][0]) < tolower(s_buf->entry[smallest][0]) &&
                    s_buf->entry[j][0] && s_buf->entry[smallest][0])
                smallest = j;

            if (tolower(s_buf->entry[j][0]) == tolower(s_buf->entry[smallest][0]) && s_buf->entry[j] && s_buf->entry[smallest][0])
                for (u16 k = 1; k < NAME_MAX - 1 && s_buf->entry[j][k - 1] && s_buf->entry[smallest][k - 1]; ++k)
                {
                    if (tolower(s_buf->entry[j][k]) < tolower(s_buf->entry[smallest][k]))
                    {
                        smallest = j;
                        break;
                    }
                }
        }

        swap_strings(s_buf->entry[i], s_buf->entry[smallest]);
    }
}

