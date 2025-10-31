#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <engine/h/core.h>
#include "main.h"

enum ContainerState
{
    STATE_CONTR_CHEST,
    STATE_CONTR_CRAFTING_TABLE,
    STATE_CONTR_FURNACE,
    STATE_CONTR_INVENTORY_SURVIVAL,
    STATE_CONTR_INVENTORY_SANDBOX,
    STATE_CONTR_COUNT,
}; /* ContainerState */

void player_state_update(Render *render, Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v);

void player_camera_movement_update(Render *render, Player *player,
        b8 use_mouse);

void player_target_update(Player *player);
void set_player_pos(Player *player, f64 x, f64 y, f64 z);
void set_player_block(Player *player, i32 x, i32 y, i32 z);
void player_kill(Player *player);
void player_respawn(Player *player);
void update_gravity(Render *render, Player *player);
void player_collision_update(Player *player, Chunk **chunk);

f64 get_time_ms(void);
b8 get_timer(f64 *time_start, f32 interval);

#endif /* GAME_LOGIC_H */
