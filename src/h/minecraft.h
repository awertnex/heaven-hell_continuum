#ifndef MINECRAFT_H
#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include <types.h>
#include "logic.h"

#define WIDTH		1280
#define HEIGHT		720
#define MARGIN		20

enum DebugStates
{
	ModeDebug =		1,
	ModeCollide =	0,
	ModeGravity =	1,
}; /* DebugStates */

typedef struct window
{
	v2f32 scl;
} window;

// ---- declarations -----------------------------------------------------------
extern window win;
extern f64 start_time;
#define dt (get_time_ms() - start_time)

extern u16 state;
extern u8 state_menu_depth;
extern v3i32 target_coordinates_feet; /*temp*/

// ---- signatures -------------------------------------------------------------
void	main_init();
void	main_loop();
void	main_close();
void	listen(player *player);
void	listen_menus(player *player);
void	test();

// ---- platform ---------------------------------------------------------------
#if defined __linux__
#define PLATFORM_LINUX

#elif defined _WIN32 || defined _WIN64 || defined __CYGWIN__s
#define PLATFORM_WINDOWS

#endif // PLATFORM

#define	MINECRAFT_H
#endif // MINECRAFT_H
