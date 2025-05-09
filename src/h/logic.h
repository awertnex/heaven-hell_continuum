#ifndef MC_C_LOGIC_H

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/raymath.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "../engine/h/defines.h"
#include "setting.h"

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
#define PLAYER_SPEED_FLY_FAST   30.0f
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

    Camera3D camera;
    f32 camera_distance;            // for camera collision detection
    Camera3D camera_debug_info;

    v3i32 delta_pos;                // for collision tunneling prevention
    v3i32 delta_target;
    v2i16 chunk;                    // current chunk player is in
    v2i16 delta_chunk;              // previous chunk player was in

    v3i32 spawn_point;
} Player;

// ---- states -----------------------------------------------------------------
enum GameStates
{
    STATE_ACTIVE =                  0x0001,
    STATE_PAUSED =                  0x0002,
    STATE_SUPER_DEBUG =             0x0004,
    STATE_DEBUG =                   0x0008,
    STATE_DEBUG_MORE =              0x0010,
    STATE_HUD =                     0x0020,
    STATE_FULLSCREEN =              0x0040,
    STATE_WORLD_LOADED =            0x0080,
    STATE_PARSE_CURSOR =            0x0100,
}; /* GameStates */

enum ContainerStates
{
    CONTR_ANVIL =                   0x00000001,
    CONTR_BEACON =                  0x00000002,
    CONTR_BLAST_FURNACE =           0x00000004,
    CONTR_BREWING_STAND =           0x00000008,
    CONTR_CARTOGRAPHY_TABLE =       0x00000010,
    CONTR_CHEST =                   0x00000020,
    CONTR_CHEST_LARGE =             0x00000040,
    CONTR_CRAFTING_TABLE =          0x00000080,
    CONTR_DISPENSER =               0x00000100,
    CONTR_ENCHANTING_TABLE =        0x00000200,
    CONTR_FURNACE =                 0x00000400,
    CONTR_GAMEMODE_SWITCHER =       0x00000800,
    CONTR_GRINDSTONE =              0x00001000,
    CONTR_HOPPER =                  0x00002000,
    CONTR_HORSE =                   0x00004000,
    CONTR_INVENTORY =               0x00008000,
    CONTR_LEGACY_SMITHING =         0x00010000,
    CONTR_LOOM =                    0x00020000,
    CONTR_SMITHING =                0x00040000,
    CONTR_SMOKER =                  0x00080000,
    CONTR_STONECUTTER =             0x00100000,
    CONTR_VILLAGER =                0x00200000,
    CONTR_TAB_INVENTORY =           0x00400000,
    CONTR_TAB_ITEMS =               0x00800000,
    CONTR_TAB_ITEMS_SEARCH =        0x01000000,
}; /* ContainerStates */

enum PlayerStates
{
    STATE_CAN_JUMP =                0x0001,
    STATE_SNEAKING =                0x0002,
    STATE_SPRINTING =               0x0004,
    STATE_FLYING =                  0x0008,
    STATE_SWIMMING =                0x0010,
    STATE_MENU_OPEN =               0x0020,
    STATE_HUNGRY =                  0x0040,
    STATE_FALLING =                 0x0080,
    STATE_DOUBLE_PRESS =            0x0100,
    STATE_PARSE_TARGET =            0x0200,
    STATE_VELOCITY_DIRTY =          0x0400,
    STATE_DEAD =                    0x0800,
    STATE_CHUNK_BUF_DIRTY =         0x1000,
}; /* PlayerStates */

// ---- declarations -----------------------------------------------------------
extern Player lily;
extern Vector2 mouse_delta;

// ---- signatures -------------------------------------------------------------
bool get_double_press(Player *player, KeyboardKey key);
void update_player_states(Player *player);
void update_camera_movements_player(Player *player);
void update_camera_movements_debug_info(Camera3D *camera, Player *player);
void set_player_pos(Player *player, f32 x, f32 y, f32 z);
void set_player_block(Player *player, i32 x, i32 y, i32 z);
void kill_player(Player *player);
void respawn_player(Player *player);
b8 check_delta_target(Vector3 *coordinates, v3i32 *delta_target);
b8 is_range_within_ff(f32 *pos, f32 start, f32 end);
b8 is_range_within_v2ff(v2f32 *pos, v2f32 start, v2f32 end);
b8 is_range_within_v3fi(Vector3 *pos, v3i32 start, v3i32 end);
b8 is_distance_within(u16 distance, v2i32 start, v2i32 end);
b8 is_ray_intersect(Player *player); //TODO: make better ray_intersect checking
void give_gravity(Player *player);
void update_collision_static(Player *player);
f64 get_time_ms();
b8 get_timer(f64 *time_start, f32 interval);

void draw_default_grid(Color x, Color y, Color z);

#define MC_C_LOGIC_H
#endif

