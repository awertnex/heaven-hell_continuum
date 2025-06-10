#if defined(__linux__) || defined(__linux)
#ifndef PLATFORM_LINUX_INCLUDE
#define PLATFORM_LINUX_INCLUDE

#include "h/main.h"
#include "h/setting.h"

#define MC_C_HOME "HOME"
#define MC_C_ROAMING ""

#include "h/assets.h"
#include "h/chunking.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/logic.h"
#include "h/super_debugger.h"

int mc_c_mkdir(str *path, u16 mode)
{
    return mkdir(path, mode);
}

void update_debug_strings(Player *player)
{
    snprintf(str_fps, 16,                "FPS: %d",                 GetFPS());
    snprintf(str_player_pos, 32,          "XYZ: %.2f %.2f %.2f",    player->pos.x, player->pos.y, player->pos.z);
    snprintf(str_player_block, 32,        "BLOCK: %.0f %.0f %.0f",  floorf(player->pos.x), floorf(player->pos.y), floorf(player->pos.z));
    snprintf(str_player_chunk, 48,        "CHUNK: %d %d %d",
            (i16)floorf(player->pos.x / CHUNK_DIAMETER),
            (i16)floorf(player->pos.y / CHUNK_DIAMETER),
            (i16)floorf(player->pos.z / CHUNK_DIAMETER));
    snprintf(str_player_direction, 32,    "YAW: %.1f PITCH: %.1f",  player->yaw, player->pitch);
    snprintf(str_block_count, 32,         "BLOCKS: %ld",            globals.block_count);
    snprintf(str_quad_count, 32,          "QUADS: %ld",             globals.quad_count);
    snprintf(str_tri_count, 32,           "TRIS: %ld",              globals.quad_count * 2);
    snprintf(str_vertex_count, 32,        "VERTICES: %ld",          globals.quad_count * 6);
}

#endif /* PLATFORM_LINUX_INCLUDE */
#endif /* PLATFORM_LINUX */

