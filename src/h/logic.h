#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "../engine/h/core.h"

/* ---- section: player defaults -------------------------------------------- */

typedef struct Player
{
    str name[100];                  /* player in-game name */
    v3f32 pos;                      /* player processed raw_pos */
    v3f32 raw_pos;                  /* player current coordinates in world */
    v3f32 pos_lerp_speed;
    v3f32 target;                   /* player arm (or whatever) */
    v3f32 scl;                      /* player size for collision detection */
    v3f32 collision_check_start;
    v3f32 collision_check_end;
    f32 pitch, yaw;                 /* for player camera direction and target */
    f32 sin_pitch;                  /* processed player pitch sine angle */
    f32 cos_pitch;                  /* processed player pitch cosine angle */
    f32 sin_yaw;                    /* processed player yaw sine angle */
    f32 cos_yaw;                    /* processed player yaw cosine angle */
    f32 eye_height;                 /* height of player camera, usually */
    v3f32 vel;                      /* velocity */
    f32 mass;                       /* for gravity influence */
    f32 movement_speed;             /* depends on enum: PlayerFlags */
    u64 container_state;            /* enum: ContainerFlags */
    u8 perspective;                 /* camera perspective mode */
    u16 state;                      /* enum: PlayerFlags */

    Camera camera;
    f32 camera_distance;            /* for camera collision detection */

    /* TODO: do player overflow */
    u8 overflow;                    /* player at world edge, enum: PlayerFlags */
    v3i32 delta_pos;                /* for collision tunneling prevention */
    v3i32 delta_target;             /* player arm snapped to grid */
    v3i16 chunk;                    /* current chunk player is in */
    v3i16 delta_chunk;              /* previous chunk player was in */

    v3i32 spawn_point;
} Player;

/* ---- section: flags ------------------------------------------------------ */

enum StateFlags
{
    FLAG_ACTIVE =                   0x0001,
    FLAG_PAUSED =                   0x0002,
    FLAG_PARSE_CURSOR =             0x0004,
    FLAG_HUD =                      0x0008,
    FLAG_DEBUG =                    0x0010,
    FLAG_DEBUG_MORE =               0x0020,
    FLAG_SUPER_DEBUG =              0x0040,
    FLAG_FULLSCREEN =               0x0080,
    FLAG_MENU_OPEN =                0x0100,
    FLAG_DOUBLE_PRESS =             0x0200,
    FLAG_PARSE_TARGET =             0x0400,
    FLAG_WORLD_LOADED =             0x0800,
    FLAG_CHUNK_BUF_DIRTY =          0x1000,
}; /* StateFlags */

enum PlayerFlags
{
    FLAG_CAN_JUMP =                 0x0001,
    FLAG_SNEAKING =                 0x0002,
    FLAG_SPRINTING =                0x0004,
    FLAG_FLYING =                   0x0008,
    FLAG_SWIMMING =                 0x0010,
    FLAG_FALLING =                  0x0020,
    FLAG_VELOCITY_DIRTY =           0x0040,
    FLAG_HUNGRY =                   0x0080,
    FLAG_DEAD =                     0x0100,

    FLAG_OVERFLOW_X =               0x0001,
    FLAG_OVERFLOW_Y =               0x0002,
    FLAG_OVERFLOW_Z =               0x0004,
    FLAG_OVERFLOW_DIRECTION_X =     0x0008,
    FLAG_OVERFLOW_DIRECTION_Y =     0x0010,
    FLAG_OVERFLOW_DIRECTION_Z =     0x0020,
}; /* PlayerFlags */

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

/* ---- section: declarations ----------------------------------------------- */

extern Player lily;

/* ---- section: signatures ------------------------------------------------- */

void update_player(Render *render, Player *player);
void update_camera_movement_player(Render *render, Player *player);
void update_player_target(v3f32 *player_target, v3i32 *player_delta_target);

static inline void set_player_pos(Player *player, f32 x, f32 y, f32 z)
{player->raw_pos = (v3f32){x, y, z};}

static inline void set_player_block(Player *player, i32 x, i32 y, i32 z)
{player->raw_pos = (v3f32){(f32)(x + 0.5f), (f32)(y + 0.5f), (f32)(z + 0.5f)};}

void set_player_pos(Player *player, f32 x, f32 y, f32 z);
void set_player_block(Player *player, i32 x, i32 y, i32 z);
void player_kill(Player *player);
void player_respawn(Player *player);

u32 get_distance(v3i32 a, v3i32 b);
b8 is_range_within_i(i32 pos, i32 start, i32 end);
b8 is_range_within_f(f32 pos, f32 start, f32 end);
b8 is_range_within_v2f(v2f32 pos, v2f32 start, v2f32 end);
b8 is_range_within_v3i(v3i32 pos, v3i32 start, v3i32 end);
b8 is_range_within_v3fi(v3f32 pos, v3i32 start, v3i32 end);
b8 is_ray_intersect(Player *player);

void update_gravity(Render *render, Player *player);
void update_collision_static(Player *player);
f64 get_time_ms();
b8 get_timer(f64 *time_start, f32 interval);

void update_debug_strings(Player *player);

#ifdef FUCK // TODO: undef FUCK
void draw_default_grid(v4u8 x, v4u8 y, v4u8 z);
#endif // TODO: undef FUCK

#endif /* GAME_LOGIC_H */

