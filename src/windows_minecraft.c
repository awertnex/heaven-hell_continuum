#include "h/main.h"
#include "sys/stat.h"

#define MC_C_HOME "APPDATA"
#define INIT_MC_C_GRANDPATH snprintf(mc_c_grandpath, strlen(getenv("APPDATA")) + 14, "%s/Roaming/minecraft.c/", getenv("APPDATA"));

int mc_mkdir(const char *path, mode_t mode)
{
    return mkdir(path);
}
