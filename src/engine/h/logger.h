#ifndef MC_C_ENGINE_LOGGER_H
#define MC_C_ENGINE_LOGGER_H

#include "defines.h"

#define RELEASE_BUILD 0

#if RELEASE_BUILD
    #define LOGGING_DEBUG 0
    #define LOGGING_TRACE 0
#else
    #define LOGGING_DEBUG 1
    #define LOGGING_TRACE 1
#endif

enum LogLevel
{
    LOGGER_FATAL = 0,
    LOGGER_ERROR,
    LOGGER_WARNING,
    LOGGER_INFO,
    LOGGER_DEBUG,
    LOGGER_TRACE,
}; /* LogLevel */

b8 init_logger();
void close_logger();
void log_output(u8 log_level, const str* format, ...);

#define LOGFATAL(format, ...) log_output(LOGGER_FATAL, format, ##__VA_ARGS__)
#define LOGERROR(format, ...) log_output(LOGGER_ERROR, format, ##__VA_ARGS__)
#define LOGWARNING(format, ...) log_output(LOGGER_WARNING, format, ##__VA_ARGS__)
#define LOGINFO(format, ...) log_output(LOGGER_INFO, format, ##__VA_ARGS__)

#if LOGGING_DEBUG == 1
    #define LOGDEBUG(format, ...) log_output(LOGGER_DEBUG, format, ##__VA_ARGS__)
#else
    #define LOGDEBUG(format, ...)
#endif

#if LOGGING_TRACE == 1
    #define LOGTRACE(format, ...) log_output(LOGGER_TRACE, format, ##__VA_ARGS__)
#else
    #define LOGTRACE(format, ...)
#endif

#endif /* MC_C_ENGINE_LOGGER_H */

