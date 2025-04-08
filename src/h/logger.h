#pragma once

#include "defines.h"

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

b8 init_logger();
void close_logger();
void log_output(u8 logLevel, const str* message, ...);

#define LOGFATAL(message, ...) log_output(LOGGER_FATAL, message, ##__VA_ARGS__)
#define LOGWARNING(message, ...) log_output(LOGGER_WARNING, message, ##__VA_ARGS__)

#if LOGGING_ERROR == 1
    #define LOGERROR(message, ...) log_output(LOGGER_ERROR, message, ##__VA_ARGS__)
#else
    #define LOGERROR(message, ...)
#endif

#if LOGGING_INFO == 1
    #define LOGINFO(message, ...) log_output(LOGGER_INFO, message, ##__VA_ARGS__)
#else
    #define LOGINFO(message, ...)
#endif

#if LOGGING_DEBUG == 1
    #define LOGDEBUG(message, ...) log_output(LOGGER_DEBUG, message, ##__VA_ARGS__)
#else
    #define LOGDEBUG(message, ...)
#endif
