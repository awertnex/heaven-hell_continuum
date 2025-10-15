#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

#include "defines.h"

#if defined(__linux__) || defined(__linux)
#define PLATFORM_LINUX 1
#define PLATFORM "linux"
#define _PLATFORM "linux"
#define EXE ""

#define SLASH_NATIVE '/'
#define SLASH_NON_NATIVE '\\'

#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define PLATFORM_WIN 1
#define PLATFORM "win"
#define _PLATFORM "windows"
#define EXE ".exe"

#define SLASH_NATIVE '\\'
#define SLASH_NON_NATIVE '/'

#endif /* PLATFORM */

/* -- IMPLEMENTATION: platform_<PLATFORM>.c --; */
int make_dir(const str *path);

/* -- INTERNAL USE ONLY --;
 *
 * -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 * get real path.
 * 
 * path = relative path,
 * path_real = result/canonical path, ending with slash.
 *
 * return FALSE (0) on failure */
b8 _get_path_absolute(const str *path, str *path_real);

/* -- INTERNAL USE ONLY --;
 *
 * -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 * get current path of binary/executable,
 * assign allocated path string to path.
 *
 * return FALSE (0) on failure */
b8 _get_path_bin_root(str *path);

/* -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 * fork child process and execute command,
 * based on execvp().
 * 
 * cmd = command and args,
 * cmd_name = command name (for logging).
 *
 * return FALSE (0) on failure */
b8 exec(buf *cmd, str *cmd_name);

#endif /* ENGINE_PLATFORM_H */
