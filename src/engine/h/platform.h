#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

#if defined(__linux__) || defined(__linux)
    #include "../platform_linux.c"
#elif defined(__WIN32) || defined(__WIN64) || defined (__CYGWIN__)
    #include "../platform_windows.c"
#endif /* PLATFORM */

#include "defines.h"

int make_dir(str *path);

/*
 * -- INTERNAL USE ONLY --;
 * get real path;
 * returns FALSE (0) on failure;
 * 
 * path = relative path;
 * path_real = result/canonical path, ending with slash;
 */
b8 _get_path_absolute(const str *path, str *path_real);

/*
 * -- INTERNAL USE ONLY --;
 * get current path of binary/executable;
 * assign allocated path string to buf;
 * returns FALSE (0) on failure;
 */
b8 _get_path_bin_root(str *buf);

/* 
 * fork child process and execute command;
 * uses execvp();
 * returns FALSE (0) on failure;
 * 
 * cmd = command and args;
 * cmd_name = command name (for logging);
 */
b8 exec(str *const *cmd, str *cmd_name);

#endif /* ENGINE_PLATFORM_H */

