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

int mc_mkdir(const char *path, u16 mode)
{
    return mkdir(path, mode);
}

void update_debug_strings()
{
    snprintf(str_fps, 16,                       "FPS: %d",                  GetFPS());
    snprintf(str_player_position, 32,           "XYZ: %.2f %.2f %.2f",      lily.pos.x, lily.pos.y, lily.pos.z);
    snprintf(str_player_block, 32,              "BLOCK: %.0f %.0f %.0f",    floorf(lily.pos.x), floorf(lily.pos.y), floorf(lily.pos.z));
    snprintf(str_player_chunk, 32,              "CHUNK: %d %d",             (i16)floorf(lily.pos.x/CHUNK_SIZE), (i16)floorf(lily.pos.y/CHUNK_SIZE));
    snprintf(str_player_direction, 32,          "YAW: %.1f PITCH: %.1f",    lily.yaw, lily.pitch);
    snprintf(str_block_count, 32,               "BLOCKS: %ld",              block_count);
    snprintf(str_quad_count, 32,                "QUADS: %ld",               quad_count);
    snprintf(str_tri_count, 32,                 "TRIS: %ld",                quad_count*2);
    snprintf(str_vertex_count, 32,              "VERTICES: %ld",            quad_count*6);
}
