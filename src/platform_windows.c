#include <stdio.h>
#include <math.h>

#include "h/chunking.h"
#include "h/gui.h"
#include "h/logic.h"

void update_debug_strings(Player *player)
{
    snprintf(str_debug_info[STR_DEBUG_INFO_FPS], 16, "FPS: %d\n", 5);
    snprintf(str_debug_info[STR_DEBUG_INFO_PLAYER_POS], 32, "XYZ: %d %d %d\n",
            (i32)player->pos.x,
            (i32)player->pos.y,
            (i32)player->pos.z);

    snprintf(str_debug_info[STR_DEBUG_INFO_PLAYER_BLOCK], 32, "BLOCK: %.0f %.0f %.0f\n",
            floorf(player->pos.x),
            floorf(player->pos.y),
            floorf(player->pos.z));

    snprintf(str_debug_info[STR_DEBUG_INFO_PLAYER_CHUNK], 48, "CHUNK: %d %d %d\n",
            (i32)floorf(player->pos.x / CHUNK_DIAMETER),
            (i32)floorf(player->pos.y / CHUNK_DIAMETER),
            (i32)floorf(player->pos.z / CHUNK_DIAMETER));

    snprintf(str_debug_info[STR_DEBUG_INFO_PLAYER_DIRECTION], 32, "YAW: %d PITCH: %d\n",
            (i32)player->yaw,
            (i32)player->pitch);

    snprintf(str_block_count, 32, "BLOCKS: %lld", globals.block_count);
    snprintf(str_quad_count, 32, "QUADS: %lld", globals.quad_count);
    snprintf(str_tri_count, 32, "TRIS: %lld", globals.quad_count * 2);
    snprintf(str_vertex_count, 32, "VERTICES: %lld", globals.quad_count * 6);
}

