#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <engine/h/core.h>
#include "main.h"

enum ContainerStates
{
    STATE_CONTR_ANVIL,
    STATE_CONTR_BEACON,
    STATE_CONTR_BLAST_FURNACE,
    STATE_CONTR_BREWING_STAND,
    STATE_CONTR_CARTOGRAPHY_TABLE,
    STATE_CONTR_CHEST,
    STATE_CONTR_CHEST_LARGE,
    STATE_CONTR_CRAFTING_TABLE,
    STATE_CONTR_DISPENSER,
    STATE_CONTR_ENCHANTING_TABLE,
    STATE_CONTR_FURNACE,
    STATE_CONTR_GAMEMODE_SWITCHER,
    STATE_CONTR_GRINDSTONE,
    STATE_CONTR_HOPPER,
    STATE_CONTR_HORSE,
    STATE_CONTR_INVENTORY,
    STATE_CONTR_LEGACY_SMITHING,
    STATE_CONTR_LOOM,
    STATE_CONTR_SMITHING,
    STATE_CONTR_SMOKER,
    STATE_CONTR_STONECUTTER,
    STATE_CONTR_VILLAGER,
    STATE_CONTR_TAB_INVENTORY,
    STATE_CONTR_TAB_ITEMS,
    STATE_CONTR_TAB_ITEMS_SEARCH,
}; /* ContainerStates */

void update_player(Render *render, Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v);

void update_camera_movement_player(Render *render, Player *player,
        b8 use_mouse);

void update_player_target(Player *player);

static inline void
set_player_pos(Player *player, f64 x, f64 y, f64 z)
{
    player->pos = (v3f64){x, y, z};
    player->pos_smooth = player->pos;
}

static inline void
set_player_block(Player *player, i32 x, i32 y, i32 z)
{
    player->pos =
        (v3f64){(f64)(x) + 0.5f, (f64)(y) + 0.5f, (f64)(z) + 0.5f};
    player->pos_smooth = player->pos;
}

void player_kill(Player *player);
void player_respawn(Player *player);
b8 is_ray_intersect(Player *player);
void update_gravity(Render *render, Player *player);
void update_collision_static(Player *player);

void wrap_coordinates(Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v);

f64 get_time_ms(void);
b8 get_timer(f64 *time_start, f32 interval);

#ifdef FUCK // TODO: undef FUCK
void draw_default_grid(v4u8 x, v4u8 y, v4u8 z);
#endif // TODO: undef FUCK

#endif /* GAME_LOGIC_H */
