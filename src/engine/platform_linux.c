#if defined(__linux__) || defined(__linux)
#ifndef PLATFORM_LINUX_INCLUDE_ENGINE
#define PLATFORM_LINUX_INCLUDE_ENGINE

#include <linux/limits.h>

#include "h/dir.h"

int make_dir(str *path, u16 mode)
{
    return mkdir(path, mode);
}

#endif /* PLATFORM_LINUX_INCLUDE_ENGINE */
#endif /* PLATFORM_LINUX_ENGINE */

