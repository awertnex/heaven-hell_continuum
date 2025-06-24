#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

#if defined(__linux__) || defined(__linux)
    #include "../platform_linux.c"
#elif defined(__WIN32) || defined(__WIN64) || defined (__CYGWIN__)
    #include "../platform_windows.c"
#endif /* PLATFORM */

#include "defines.h"

int make_dir(str *path);
str *_get_path_absolute(const str *path, str *path_real);
b8 _get_path_bin_root(str *buf);

#endif /* ENGINE_PLATFORM_H */
