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

str in_message[IN_MESSAGE_MAX] = {0};
str out_message[OUT_MESSAGE_MAX] = {0};

/* ---- section: functions -------------------------------------------------- */

b8 init_logger()
{
    // TODO: init logger
    return true;
}

void close_logger()
{
    // TODO: close logger
}

void log_output(u8 log_level, const str* format, ...)
{
    // TODO: use b8 is_error;
    //b8 is_error = log_level < 2;

    __builtin_va_list args;
    va_start(args, format);
    vsnprintf(in_message, IN_MESSAGE_MAX, format, args);
    va_end(args);

    snprintf(out_message, OUT_MESSAGE_MAX, "%s: %s", log_tag[log_level], in_message);
    fprintf(stderr, "%s%s%s", esc_code_open[log_level], out_message, esc_code_close);
}

