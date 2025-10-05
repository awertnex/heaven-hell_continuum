#ifndef ENGINE_LOGGER_H
#define ENGINE_LOGGER_H

#include "defines.h"
#include "memory.h"

#define RELEASE_BUILD 1

#if RELEASE_BUILD
    #define LOGGING_DEBUG 0
    #define LOGGING_TRACE 0
#else
    #define LOGGING_DEBUG 1
    #define LOGGING_TRACE 1
#endif /* RELEASE_BUILD */

enum LogLevel
{
    LOGLEVEL_FATAL = 0,
    LOGLEVEL_ERROR,
    LOGLEVEL_WARNING,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_TRACE,
}; /* LogLevel */

extern u32 log_level;

b8 init_logger();
void close_logger();
void log_output(u8 level, const str* format, ...);

#define LOGFATAL(format, ...) \
        log_output(LOGLEVEL_FATAL, format, ##__VA_ARGS__)

#define LOGERROR(format, ...) \
        log_output(LOGLEVEL_ERROR, format, ##__VA_ARGS__)

#define LOGWARNING(format, ...) \
        log_output(LOGLEVEL_WARNING, format, ##__VA_ARGS__)

#define LOGINFO(format, ...) \
        log_output(LOGLEVEL_INFO, format, ##__VA_ARGS__)

#if LOGGING_DEBUG == 1
    #define LOGDEBUG(format, ...) \
        log_output(LOGLEVEL_DEBUG, format, ##__VA_ARGS__)
#else
    #define LOGDEBUG(format, ...)
#endif /* LOGGING_DEBUG */

#if LOGGING_TRACE == 1
    #define LOGTRACE(format, ...) \
        log_output(LOGLEVEL_TRACE, format, ##__VA_ARGS__)
#else
    #define LOGTRACE(format, ...)
#endif /* LOGGING_TRACE */

#endif /* ENGINE_LOGGER_H */
