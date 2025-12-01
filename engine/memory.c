#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "h/diagnostics.h"
#include "h/platform.h"
#include "h/memory.h"
#include "h/limits.h"
#include "h/logger.h"

u32 _mem_alloc(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (*x != NULL)
    {
        engine_err = ERR_POINTER_NOT_NULL;
        return engine_err;
    }

    *x = calloc(1, size);
    if (*x == NULL)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_ALLOC_FAIL,
                "%s[%p] Memory Allocation Failed, Process Aborted\n",
                name, NULL);
        return engine_err;
    }
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Allocated [%lldB]\n",
            name, *x, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_alloc_memb(void **x, u64 memb, u64 size,
        const str *name, const str *file, u64 line)
{
    if (*x != NULL)
        return TRUE;

    *x = calloc(memb, size);
    if (*x == NULL)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_ALLOC_FAIL,
                "%s[%p] Memory Allocation Failed, Process Aborted\n",
                name, NULL);
        return engine_err;
    }
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Allocated [%lldB]\n",
            name, *x, memb * size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_alloc_buf(Buf *x, u64 memb, u64 size,
        const str *name, const str *file, u64 line)
{
    if (x == NULL)
    {
        LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Memory Allocation Failed, Pointer NULL\n",
                name, NULL);
        return engine_err;
    }

    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    snprintf(name_i, NAME_MAX, "%s.i", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (_mem_alloc_memb((void*)&x->i,
                memb, sizeof(str*), name_i, file, line) != ERR_SUCCESS)
        return engine_err;

    if (_mem_alloc_memb((void*)&x->buf,
                memb, size, name_buf, file, line) != ERR_SUCCESS)
    {
        _mem_free((void*)&x->i, memb * sizeof(str*), name_i, file, line);
        return engine_err;
    }

    u64 i = 0;
    for (; i < memb; ++i)
        x->i[i] = x->buf + (i * size);
    x->memb = memb;
    x->size = size;
    x->loaded = TRUE;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_realloc(void **x, u64 size,
        const str *name, const str *file, u64 line)
{
    if (*x == NULL)
    {
        LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Memory Reallocation Failed, Pointer NULL\n",
                name, NULL);
        return engine_err;
    }

    void *temp = realloc(*x, size);
    if (temp == NULL)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_REALLOC_FAIL,
                "%s[%p] Memory Reallocation Failed, Process Aborted\n",
                name, *x);
        return engine_err;
    }
    *x = temp;
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Reallocated [%lldB]\n",
            name, *x, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_realloc_memb(void **x, u64 memb, u64 size,
        const str *name, const str *file, u64 line)
{
    if (*x == NULL)
    {
        LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Memory Reallocation Failed, Pointer NULL\n",
                name, NULL);
        return engine_err;
    }
    void *temp = realloc(*x, memb * size);
    if (temp == NULL)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_REALLOC_FAIL,
                "%s[%p] Memory Reallocation Failed, Process Aborted\n",
                name, *x);
        return engine_err;
    }
    *x = temp;
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Reallocated [%lldB]\n",
            name, *x, memb * size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void _mem_free(void **x, u64 size,
        const str *name, const str *file, u64 line)
{
    if (*x == NULL)
        return;

    void *temp = *x;
    mem_clear(x, size, name);
    free(*x);
    *x = NULL;
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name, temp);
}

void _mem_free_buf(Buf *x, const str *name, const str *file, u64 line)
{
    if (x == NULL) return;
    void *temp = NULL;
    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    snprintf(name_i, NAME_MAX, "%s.i", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (x->i != NULL)
    {
        temp = x->i;
        mem_clear((void*)&x->i, x->memb * sizeof(str*), name_i);
        free(x->i);
        LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n",
                name_i, temp);
    }
    if (x->buf != NULL)
    {
        temp = x->buf;
        mem_clear((void*)&x->i, x->memb * sizeof(str*), name_buf);
        free(x->buf);
        LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n",
                name_buf, temp);
    }
    *x = (Buf){NULL};
}

u32 _mem_clear(void **x, u64 size,
        const str *name, const str *file, u64 line)
{
    if (*x == NULL)
    {
        engine_err = ERR_POINTER_NULL;
        return engine_err;
    }

    memset(*x, 0, size);
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Cleared [%lldB]\n",
            name, *x, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void print_bits(u64 x, u8 bit_count)
{
    while(bit_count--)
        putchar('0' + ((x >> bit_count) & 1));
    putchar('\n');
}

void swap_bits(char *c1, char *c2)
{
    u8 i = 0;
    for (; i < 8; ++i)
    {
        if (((*c1 >> i) & 1) == ((*c2 >> i) & 1))
            continue;

        *c1 ^= (1 << i);
        *c2 ^= (1 << i);
    }
}

void swap_bits_u8(u8 *a, u8 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void swap_bits_u32(u32 *a, u32 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void swap_bits_u64(u64 *a, u64 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

#if 0
void sort_buf(Buf *buf) /* TODO: fucking fix this */
{
    for (u16 i = 0, smallest = 0; i < buf->memb - 1 && buf->i[i] != NULL; ++i)
    {
        smallest = i;
        for (u64 j = i + 1; j < buf->memb && buf->i[j] != NULL; ++j)
        {
            char cmp0 = tolower(buf->i[j] + 0);
            char cmp1 = tolower(buf->i[smallest] + 0);
            if (cmp0 < cmp1 && buf->i[j][0] && buf->i[smallest][0])
                smallest = j;

            if (cmp0 == cmp1 && buf->i[j] && cmp1)
                for (u16 k = 1; k < NAME_MAX - 1 && buf->i[j][k - 1] && buf->i[smallest][k - 1]; ++k)
                {
                    if (tolower(buf->i[j] + k) < tolower(buf->i[smallest] + k))
                    {
                        smallest = j;
                        break;
                    }
                }
        }

        swap_strings(buf->i[i], buf->i[smallest]);
    }
}
#endif
