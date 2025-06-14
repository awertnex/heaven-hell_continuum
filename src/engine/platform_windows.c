#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#ifndef PLATFORM_WINDOWS_INCLUDE_ENGINE
#define PLATFORM_WINDOWS_INCLUDE_ENGINE

#include "h/core.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/math.h"

int make_dir(str *path)
{
    return mkdir(path);
}

#endif /* PLATFORM_WINDOWS_INCLUDE_ENGINE */
#endif /* PLATFORM_WINDOWS_ENGINE */
