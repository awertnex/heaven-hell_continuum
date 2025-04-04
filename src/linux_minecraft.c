#include "h/main.h"
#include "sys/stat.h"

#define MC_C_HOME "HOME"
#define INIT_MC_C_GRANDPATH snprintf(mc_c_grandpath, strlen(getenv("HOME")) + 14, "%s/minecraft.c/", getenv("HOME"));

int mc_mkdir(const char *path, mode_t mode)
{
    return mkdir(path, mode);
}
