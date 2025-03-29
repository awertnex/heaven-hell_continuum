#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <defines.h>

#define LOG(x, ...) fprintf(stderr, "--%s: %s\n", log_level[x], __VA_ARGS__)

enum LogLevel
{
    LOGGER_FATAL = 0,
    LOGGER_WARNING = 1,
    LOGGER_ERROR = 2,
    LOGGER_INFO = 3,
}; /* LogLevel */

static str log_level[4][9] = 
{
    "FATAL",
    "WARNING",
    "ERROR",
    "INFO",
};
