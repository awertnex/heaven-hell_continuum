#include <stdio.h>
#include <stdarg.h>

#include "h/logger.h"

static str log_tag[5][9] = 
{
    "FATAL",
    "WARNING",
    "ERROR",
    "INFO",
    "DEBUG",
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

void log_output(u8 log_level, const str* message, ...)
{
    b8 is_error = log_level < 2;
    str in_message[4096] = {0};
    str out_message[4096] = {0};

    __builtin_va_list args;
    va_start(args, message);
    vsnprintf(in_message, 4096, message, args);
    va_end(args);

    snprintf(out_message, 4096, "  %s: %s\n", log_tag[log_level], in_message);
    printf("%s", out_message);
}
