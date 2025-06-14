#include <stdio.h>
#include <math.h>

#include "h/setting.h"

#include "h/assets.h"
#include "h/chunking.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/logic.h"
#include "h/super_debugger.h"

void update_debug_strings(Player *player)
{
    snprintf(str_debug_info[STR_DEBUG_INFO_FPS], 16, "FPS: %d", GetFPS());
    snprintf(str_debug_info[STR_DEBUG_INFO_PLAYER_POS], 32, "XYZ: %.2f %.2f %.2f", player->pos.x, player->pos.y, player->pos.z);
    snprintf(str_debug_info[STR_DEBUG_INFO_PLAYER_BLOCK], 32, "BLOCK: %.0f %.0f %.0f", floorf(player->pos.x), floorf(player->pos.y), floorf(player->pos.z));
    snprintf(str_debug_info[STR_DEBUG_INFO_PLAYER_CHUNK], 48, "CHUNK: %d %d %d",
            (i16)floorf(player->pos.x / CHUNK_DIAMETER),
            (i16)floorf(player->pos.y / CHUNK_DIAMETER),
            (i16)floorf(player->pos.z / CHUNK_DIAMETER));
    snprintf(str_debug_info[STR_DEBUG_INFO_PLAYER_DIRECTION], 32, "YAW: %.1f PITCH: %.1f", player->yaw, player->pitch);
    snprintf(str_block_count, 32, "BLOCKS: %ld", globals.block_count);
    snprintf(str_quad_count, 32, "QUADS: %ld", globals.quad_count);
    snprintf(str_tri_count, 32, "TRIS: %ld", globals.quad_count * 2);
    snprintf(str_vertex_count, 32, "VERTICES: %ld", globals.quad_count * 6);
}

