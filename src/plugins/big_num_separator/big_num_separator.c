/*!
 *  @file big_num_separator.c
 *  @author Lily Awertnex
 *
 *  big number separator - v3 - Sep 2025
 *
 *  separating numbers with four or more decimal places with commas.
 */

#include "deps/fossil/common/api.h" /* for _GNU_SOURCE */
#include "deps/fossil/common/limits.h"

#include "big_num_separator.h"

#include <stdio.h>
#include <inttypes.h>

i64 begin_count_offset(i64 len)
{
    i64 incr = 0;
    i64 offset = 0;
    for (incr = 0; incr < len; incr++)
    {
        offset++;
        if (offset == 4)
            offset = 1;
    }
    return offset;
}

str *big_num_separator_i64(i64 n)
{
    str container[FSL_ID_CAP];
    static str result[FSL_ID_CAP];
    i64 len = 0;
    i64 lengthen = 0;
    i64 incr = 0;
    i64 reset = 0;
    i64 offset = 0;

    snprintf(container, FSL_ID_CAP, "%"PRId64, n);

    while (n > 0)
    {
        n /= 10;
        len++;
    }

    lengthen = 0;
    reset = begin_count_offset(len);
    offset = 0;
    for (incr = 0; incr <= len + lengthen; incr++)
    {
        result[incr] = container[incr - offset];
        if (reset == 1 && incr < len)
        {
            incr++;
            result[incr] = ',';
            offset++;
            reset = 4;
            lengthen++;
        }
        reset--;
    }
    result[len + lengthen] = 0;

    return result;
}
