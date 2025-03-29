#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <defines.h>

#define RELEASE_BUILD 0

#if RELEASE_BUILD
    #define LOGGING_ERROR 0
    #define LOGGING_INFO 0
    #define LOGGING_DEBUG 0
#else
    #define LOGGING_ERROR 1
    #define LOGGING_INFO 1
    #define LOGGING_DEBUG 1
#endif

enum LogLevel
{
    LOGGER_FATAL = 0,
    LOGGER_WARNING = 1,
    LOGGER_ERROR = 2,
    LOGGER_INFO = 3,
    LOGGER_DEBUG = 4,
}; /* LogLevel */

static str log_level[5][9] = 
{
    "FATAL",
    "WARNING",
    "ERROR",
    "INFO",
    "DEBUG",
};

b8 init_logger();
void close_logger();
void log(u8 log_level, const str *message, ...);

#define LOGFATAL(message, ...) log(LOGGER_FATAL, message, ##__VA_ARGS__)
#define LOGWARNING(message, ...) log(LOGGER_WARNING, message, ##__VA_ARGS__)

#if LOGGING_ERROR == 1
    #define LOGERROR(message, ...) log(LOGGER_ERROR, message, ##__VA_ARGS__)
#else
    #define LOGERROR(message, ...)
#endif

#if LOGGING_INFO == 1
    #define LOGINFO(message, ...) log(LOGGER_INFO, message, ##__VA_ARGS__)
#else
    #define LOGINFO(message, ...)
#endif

#if LOGGING_DEBUG == 1
    #define LOGDEBUG(message, ...) log(LOGGER_DEBUG, message, ##__VA_ARGS__)
#else
    #define LOGDEBUG(message, ...)
#endif
