#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <defines.h>

#define LOG(x, ...) fprintf(stderr, "--%d: %s\n", log_level[x], __VA_ARGS__)

enum LogLevel
{
    FATAL = 0,
    WARNING = 1,
    ERROR = 2,
    INFO = 3,
}; /* LogLevel */

static str log_level[4][9] = 
{
    "FATAL",
    "WARNING",
    "ERROR",
    "INFO",
};
