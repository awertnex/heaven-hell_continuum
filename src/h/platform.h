#ifndef GAME_PLATFORM_H
#define GAME_PLATFORM_H

#if defined(__linux__) || defined(__linux)
    #define PATH_HOME "HOME"
    #define PATH_ROAMING ""
    //#include "../platform_linux.c" /*temp off*/

#elif defined(__WIN32) || defined(__WIN64) || defined (__CYGWIN__)
    #define PATH_HOME "APPDATA"
    #define PATH_ROAMING "Roaming/" /* TODO: test if ROAMING is correct */
    #include "../platform_windows.c"
#endif /* PLATFORM */

#endif /* GAME_PLATFORM_H */

