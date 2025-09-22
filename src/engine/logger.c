#include <stdio.h>
#include <stdarg.h>

#include "h/defines.h"
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

str in_message[IN_STRING_MAX] = {0};
str out_message[OUT_STRING_MAX] = {0};

/* ---- section: functions -------------------------------------------------- */

b8 init_logger()
{
    // TODO: init logger
    return TRUE;
}

void close_logger()
{
    // TODO: close logger
}

void log_output(u8 level, const str* format, ...)
{
    if (level > log_level) return;

    __builtin_va_list args;
    va_start(args, format);
    vsnprintf(in_message, IN_STRING_MAX, format, args);
    va_end(args);

    snprintf(out_message, OUT_STRING_MAX, "%s: %s", log_tag[level], in_message);
    fprintf(stderr, "%s%s%s", esc_code_open[level], out_message, esc_code_close);
}

