#include <stdio.h>
#include <stdarg.h>

#include "h/logger.h"

static str log_tag[][16] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "TRACE",
};
str escape_open[][16] =
{
    "\033[1;0;31m",
    "\033[1;0;91m",
    "\033[1;0;95m",
    "\033[0m",
    "\033[0m",
    "\033[0m",
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
    str escape_close[8] = "\033[0m";

    __builtin_va_list args;
    va_start(args, format);
    vsnprintf(in_message, 4096, format, args);
    va_end(args);

    snprintf(out_message, 6144, "%s: %s", log_tag[log_level], in_message);
    fprintf(stderr, "%s%s%s", escape_open[log_level], out_message, escape_close);
}

