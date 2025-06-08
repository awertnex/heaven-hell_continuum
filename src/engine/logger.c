#include <stdio.h>
#include <stdarg.h>

#include "h/logger.h"

static str log_tag[6][13] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "TRACE",
};

b8 init_logger()
{
    //TODO: init logger
    return true;
}

void close_logger()
{
    //TODO: close logger
}

void log_output(u8 log_level, const str* format, ...)
{
    //TODO: use b8 is_error;
    //b8 is_error = log_level < 2;
    str in_message[4096] = {0};
    str out_message[8192] = {0};

    __builtin_va_list args;
    va_start(args, format);
    vsnprintf(in_message, 4096, format, args);
    va_end(args);

    snprintf(out_message, 6144, "%s: %s", log_tag[log_level], in_message);
    fprintf(stderr, "%s", out_message);
}

