#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#include "h/defines.h"
#include "h/limits.h"
#include "h/logger.h"

#if RELEASE_BUILD
u32 log_level = LOGLEVEL_ERROR;
#else
u32 log_level = LOGLEVEL_TRACE;
#endif /* RELEASE_BUILD */

static str log_tag[][16] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "TRACE",
};

str esc_code_open[][16] =
{
    "\033[31m",
    "\033[91m",
    "\033[95m",
    "\033[0m",
    "\033[0m",
    "\033[33m",
};

str esc_code_close[] = "\033[0m";

static str in_message[IN_STRING_MAX] = {0};
static str out_message[OUT_STRING_MAX] = {0};

b8
init_logger()
{
    /* TODO: init logger */
    return TRUE;
}

void
close_logger()
{
    /* TODO: close logger */
}

void
log_output(const str *file, u64 line, u8 level, const str* format, ...)
{
    if (level > log_level) return;

    __builtin_va_list args;
    va_start(args, format);
    vsnprintf(in_message, IN_STRING_MAX, format, args);
    va_end(args);

    snprintf(out_message, OUT_STRING_MAX, "%s%s%s:%s%"PRId64"%s:%s%s: %s%s",
            esc_code_open[level], file, esc_code_close,
            esc_code_open[level], line, esc_code_close,
            esc_code_open[level], log_tag[level], in_message, esc_code_close);

    fprintf(stderr, "%s", out_message);
}
