#ifndef MC_C_LOGIC_H

#include "../dependencies/raylib-5.5/include/raylib.h"
#include "../dependencies/raylib-5.5/include/raymath.h"
#include "../dependencies/raylib-5.5/include/rlgl.h"

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "../engine/h/defines.h"

#define GRAVITY (9.7803267715f / 100.0f)
#define MC_C_PI 3.14159265358979323846f
#define MC_C_DEG2RAD (MC_C_PI / 180.0f)     // 0.017453293f
#define MC_C_RAD2DEG (180.0f / MC_C_PI)     // 57.295779513f

#define v3izero ((v3i32){0.0f, 0.0f, 0.0f})
#define v3fzero ((v3f32){0.0e-5f, 0.0e-5f, 0.0e-5f})

// ---- player defaults --------------------------------------------------------
#define PLAYER_JUMP_HEIGHT      1.25f
#define PLAYER_SPEED_WALK       3.0f
#define PLAYER_SPEED_FLY        10.0f
#define PLAYER_SPEED_FLY_FAST   40.0f
#define PLAYER_SPEED_SNEAK      1.8f
#define PLAYER_SPEED_SPRINT     4.0f

typedef struct Player
{
    str name[100];                  // player in-game name
    Vector3 pos;                    // player current coordinates in world
    Vector3 scl;                    // player size for collision detection
    Vector3 collision_check_start;
    Vector3 collision_check_end;
    f32 pitch, yaw;                 // for player camera direction and target
    f32 sin_pitch, cos_pitch;       // processed player pitch angles
    f32 sin_yaw, cos_yaw;           // processed player yaw angles
    f32 eye_height;                 // height of player camera, usually
    v3f32 v;                        // velocity
    f32 m;                          // mass
    f32 movement_speed;             // depends on enum: PlayerStates
    f32 movement_step_length;
    u64 container_state;            // enum: ContainerStates
    u8 perspective;                 // camera perspective mode
    u16 state;                      // enum: PlayerStates

    Camera camera;
    f32 camera_distance;            // for camera collision detection
    Camera camera_debug_info;

    v3i32 delta_pos;                // for collision tunneling prevention
    v3i32 delta_target;
    v2i16 chunk;                    // current chunk player is in
    v2i16 delta_chunk;              // previous chunk player was in

    v3i32 spawn_point;
} Player;

// ---- flags ------------------------------------------------------------------
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

// ---- declarations -----------------------------------------------------------
extern Player lily;
extern Vector2 mouse_delta;

// ---- signatures -------------------------------------------------------------
bool get_double_press(KeyboardKey key);
void update_player(Player *player);
void update_camera_movements_player(Player *player);
void update_player_target(Vector3 *player_target, v3i32 *player_delta_target);
void set_player_pos(Player *player, f32 x, f32 y, f32 z);
void set_player_block(Player *player, i32 x, i32 y, i32 z);
void kill_player(Player *player);
void respawn_player(Player *player);

u32 get_distance(v2i32 a, v2i32 b);
b8 is_range_within_i(i32 pos, i32 start, i32 end);
b8 is_range_within_f(f32 pos, f32 start, f32 end);
b8 is_range_within_v2f(v2f32 pos, v2f32 start, v2f32 end);
b8 is_range_within_v3i(v3i32 pos, v3i32 start, v3i32 end);
b8 is_range_within_v3fi(v3f32 pos, v3i32 start, v3i32 end);
b8 is_ray_intersect(Player *player);

void update_gravity(Player *player);
void update_collision_static(Player *player);
f64 get_time_ms();
b8 get_timer(f64 *time_start, f32 interval);

void draw_default_grid(Color x, Color y, Color z);

#define MC_C_LOGIC_H
#endif

