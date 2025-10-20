#ifndef ENGINE_LOGGER_H
#define ENGINE_LOGGER_H

#include "defines.h"
#include "memory.h"
#include "diagnostics.h"

#define RELEASE_BUILD 0

enum LogLevel
{
    LOGLEVEL_FATAL = 0,
    LOGLEVEL_ERROR,
    LOGLEVEL_WARNING,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_TRACE,
}; /* LogLevel */

extern u32 log_level_max;
extern str *logger_buf;

/* return non-zero on failure and engine_err is set accordingly */
u32 logger_init(void);

void logger_close(void);

#define LOGFATAL(err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(__FILE__, __LINE__, \
            LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERROR(err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(__FILE__, __LINE__, \
            LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNING(err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(__FILE__, __LINE__, \
            LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define LOGINFO(format, ...) \
    _log_output(__FILE__, __LINE__, \
            LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#define LOGFATALV(file, line, err, format, ...); \
{ \
    engine_err = (u32)err; \
    _log_output(file, line, LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERRORV(file, line, err, format, ...); \
{ \
    engine_err = (u32)err; \
    _log_output(file, line, LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNINGV(file, line, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(file, line, \
            LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define LOGINFOV(file, line, format, ...) \
    _log_output(file, line, LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#if RELEASE_BUILD
    #define LOGDEBUG(format, ...)
    #define LOGTRACE(format, ...)
    #define LOGDEBUGV(file, line, format, ...)
    #define LOGTRACEV(file, line, format, ...)
#else
    #define LOGDEBUG(format, ...) \
        _log_output(__FILE__, __LINE__, \
                LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

    #define LOGTRACE(format, ...) \
        _log_output(__FILE__, __LINE__, \
                LOGLEVEL_TRACE, 0, format, ##__VA_ARGS__)

    #define LOGDEBUGV(file, line, format, ...) \
        _log_output(file, line, \
                LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

    #define LOGTRACEV(file, line, format, ...) \
        _log_output(file, line, \
                LOGLEVEL_TRACE, 0, format, ##__VA_ARGS__)
#endif /* RELEASE_BUILD */

/* -- INTERNAL USE ONLY --; */
void _log_output(const str *file, u64 line,
        u8 level, u32 error_code, const str* format, ...);

#define LOG_OUTPUT(err) \
{ \
    if (err == ERR_GLFW_RAW_MOUSE_MOTION_SUPPORT) \
    LOGINFO("%s\n", "GLFW: Raw Mouse Motion Enabled"); \
    else if (err == ERR_GLFW_RAW_MOUSE_MOTION_NOT_SUPPORT) \
    LOGERROR(err, "%s\n", "GLFW: Raw Mouse Motion Not Supported"); \
}

#define LOG_MESH_GENERATE(err, mesh_name) \
{ \
    if (err == ERR_SUCCESS) \
    LOGINFO("Mesh '%s' Generated\n", mesh_name); \
    else if (err == ERR_MESH_GENERATION_FAIL) \
    LOGERROR(ERR_MESH_GENERATION_FAIL, "Mesh '%s' Generation Failed\n", mesh_name); \
}

#endif /* ENGINE_LOGGER_H */
