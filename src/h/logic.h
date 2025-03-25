#ifndef LOGIC_H
#include <raylib.h>
#include <raymath.h>

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include <defines.h>
#include "setting.h"

#define GRAVITY (9.7803267715f/100.0f)
#define PI 3.14159265358979323846f
#define DEG2RAD (PI/180.0f)
#define RAD2DEG (180.0f/PI)

#define sqr(x) ((x)*(x))
#define v3izero ((v3i32){0, 0, 0})
#define v3fzero ((v3f32){0.0e-5f, 0.0e-5f, 0.0e-5f})

// ---- player defaults --------------------------------------------------------
#define PLAYER_JUMP_HEIGHT					1.25f
#define PLAYER_SPEED_WALK					0.035f
#define PLAYER_SPEED_FLY					0.1f
#define PLAYER_SPEED_FLY_FAST				0.4f
#define PLAYER_SPEED_SNEAK					0.013f
#define PLAYER_SPEED_SPRINT					0.07f

typedef struct player
{
	str name[100];			// player in-game name
	Vector3 pos;			// player current coordinates in world
	Vector3 scl;			// player size for collision detection
	f32 pitch, yaw;			// for player camera direction and target
	v3f32 v;				// for physics calculations
	f32 m;					// for physics calculations
	f32 movement_speed;		// depends on enum: PlayerStates
	u16 container_state;	// enum: ContainerStates
	u8 perspective;			// camera perspective mode
	u16 state;				// enum: PlayerStates

	Camera3D camera;
	f32 camera_distance;	// for camera collision detection

	v3i32 previous_target;
	v3i32 previous_pos;		// for collision tunneling prevention

	v3i32 spawn_point;
} player;

// ---- states -----------------------------------------------------------------
enum GameStates
{
	STATE_ACTIVE =					0x01,
	STATE_PAUSED =					0x02,
	STATE_SUPER_DEBUG =				0x04,
	STATE_DEBUG =					0x08,
	STATE_HUD =						0x10,
	STATE_FULLSCREEN =				0x20,
	STATE_WORLD_LOADED =			0x40,
}; /* GameStates */

enum ContainerStates
{
	STATE_INVENTORY =				0x01,
	STATE_CHEST =					0x02,
	STATE_FURNACE =					0x04,
}; /* ContainerStates */

enum PlayerStates
{
	STATE_CAN_JUMP =				0x0001,
	STATE_SNEAKING =				0x0002,
	STATE_SPRINTING =				0x0004,
	STATE_FLYING =					0x0008,
	STATE_SWIMMING =				0x0010,
	STATE_MENU_OPEN =				0x0020,
	STATE_HUNGRY =					0x0040,
	STATE_FALLING =					0x0080,
	STATE_DOUBLE_PRESS =			0x0100,
	STATE_PARSE_TARGET =			0x0200,
	STATE_PARSE_COLLISION_FEET =	0x0400, /*temp*/
}; /* PlayerStates */

// ---- declarations -----------------------------------------------------------
extern player lily;
extern v3i32 target_coordinates_feet; /*temp*/

// ---- signatures -------------------------------------------------------------
bool	get_double_press(player *player, KeyboardKey key);
void	parse_player_states(player *player);
void	give_camera_movements_player(player *player);
void	give_camera_movements_debug_info(player *player);
bool	check_target_delta_position(Vector3 *coordinates, v3i32 *previous_target);
bool	is_range_within_ff(f32 *pos, f32 start, f32 end);
bool	is_range_within_v2ff(v2f32 *pos, v2f32 start, v2f32 end);
bool	is_range_within_v3fi(Vector3 *pos, v3i32 start, v3i32 end);
void	give_gravity(player *player);
void	give_collision_static(player *player, v3i32 *target_coordinates_feet);
f64		get_time_ms();
bool	get_timer(f64 *time_start, f32 interval);

#define LOGIC_H
#endif
