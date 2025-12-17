#include <time.h>
#include <sys/time.h>

#include "h/logic.h"

u64 get_time_logic(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (u64)(tp.tv_sec + tp.tv_usec);
}

f64 get_time_f64(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (f64)tp.tv_sec + (f64)tp.tv_usec * 1e-6f;
}

b8 get_timer(f64 *time_start, f32 interval)
{
    f64 time_current = get_time_f64();
    if (time_current - *time_start >= interval)
    {
        *time_start = time_current;
        return TRUE;
    }
    return FALSE;
}
