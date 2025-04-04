#include <sys/stat.h>

#include "h/main.h"
#include "h/setting.h"

#define MC_C_HOME "HOME"

#include "dir.c"
#include "gui.c"
#include "chunking.c"
#include "logic.c"
#include "assets.c"
#include "keymaps.c"
#include "logger.c"
#include "super_debugger.c"

int mc_mkdir(const char *path, mode_t mode)
{
    return mkdir(path, mode);
}
