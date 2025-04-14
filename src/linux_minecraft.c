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
#include "rendering.c"
#include "super_debugger.c"
#include "engine/logger.c"

int mc_mkdir(const char *path, u16 mode)
{
    return mkdir(path, mode);
}

void update_debug_strings()
{
    snprintf(strFPS, 16,                       "FPS: %d",                  GetFPS());
    snprintf(strPlayerPos, 32,           "XYZ: %.2f %.2f %.2f",      lily.pos.x, lily.pos.y, lily.pos.z);
    snprintf(strPlayerBlock, 32,              "BLOCK: %.0f %.0f %.0f",    floorf(lily.pos.x), floorf(lily.pos.y), floorf(lily.pos.z));
    snprintf(strPlayerChunk, 32,              "CHUNK: %d %d",             (i16)floorf(lily.pos.x/CHUNK_SIZE), (i16)floorf(lily.pos.y/CHUNK_SIZE));
    snprintf(strPlayerDirection, 32,          "YAW: %.1f PITCH: %.1f",    lily.yaw, lily.pitch);
    snprintf(strBlockCount, 32,               "BLOCKS: %ld",              blockCount);
    snprintf(strQuadCount, 32,                "QUADS: %ld",               quadCount);
    snprintf(strTriCount, 32,                 "TRIS: %ld",                quadCount*2);
    snprintf(strVertexCount, 32,              "VERTICES: %ld",            quadCount*6);
}

