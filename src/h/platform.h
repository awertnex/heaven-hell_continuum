#ifndef GAME_PLATFORM_H
#define GAME_PLATFORM_H

#if defined(__linux__) || defined(__linux)
    #include "../platform_linux.c"
#elif defined(__WIN32) || defined(__WIN64) || defined (__CYGWIN__)
    #include "../platform_windows.c"
#endif /* PLATFORM */

#endif /* GAME_PLATFORM_H */

