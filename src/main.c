/* ==== section table ==========================================================

_section_instance_directory_map ================================================
_section_main ==================================================================
_section_listeners_&_input =====================================================
_section_testing ===============================================================
*/

// TASKS:    [!]: done!    [x]: deleted
// [!] display Chunk coordinates
// [x] convert world coordinates to Chunk index
// [x] convert Chunk index to world coordinates
// [!] detect targeted block
// [!] place blocks
// [!] break blocks
// [ ] figure out delta time
// [ ] detect new chunk, allocate memory and spawn accordingly
// [x] print chunk index's memory address at target on MOUSE_BUTTON_MIDDLE
// [ ] fix seg fault when player target enters non-allocated chunk area
// [ ] fix funky chunk states shifting away by 1 unit each chunk
// [ ] change chunk_buff allocation from stack to heap + access using pointer arithmetic

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <pthread.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "h/main.h"
#include "h/setting.h"
#include "h/gui.h"
#include "h/chunking.h"
#include "h/logic.h"
#include "h/assets.h"
#include "h/keymaps.h"
#include "h/super_debugger.h"
#include "tests/test.c"

// ---- variables --------------------------------------------------------------
window win =
{
	.scl = {WIDTH, HEIGHT},
};
f64 start_time = 0;
u16 state = 0;
u8 state_menu_depth = 0;

settings setting =
{
	.reach_distance =		SETTING_REACH_DISTANCE_MAX,
	.fov =					SETTING_FOV_DEFAULT,
	.mouse_sensitivity =	SETTING_MOUSE_SENSITIVITY_DEFAULT,
	.render_distance =		SETTING_RENDER_DISTANCE_DEFAULT,
	.gui_scale =			SETTING_GUI_SCALE_DEFAULT,
};

// =============================================================================
// _section_instance_directory_map =============================================
// =============================================================================

FILE *instance;
void init_instance_dir(str **instance_name)
{
	if (mkdir(*instance_name, 0775))
	{
		instance = fopen(*instance_name, "r+");
		printf("Instance Opened: %s\n", *instance_name);
	}
	else
	{
		instance = fopen(*instance_name, "r+");
		printf("Instance Created: %s\n", *instance_name);
	}
}

// =============================================================================
// _section_main ===============================================================
// =============================================================================

void main_init()
{
	if (ModeDebug) printf("debug mode: on");
	InitWindow(WIDTH, HEIGHT, "minecraft.c");
	SetWindowPosition((GetMonitorWidth(0)/2) - (WIDTH/2), (GetMonitorHeight(0)/2) - (HEIGHT/2));
	//TODO: fix fullscreen
	SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowState(FLAG_MSAA_4X_HINT);
	SetWindowState(FLAG_WINDOW_HIGHDPI);
	SetWindowMinSize(200, 150);
	hide_cursor;
	center_cursor;

	setting.render_distance = SETTING_RENDER_DISTANCE_MAX; //temp

	init_chunking();
	init_texture_layouts();
	init_textures();
	init_gui();
	init_super_debugger();

	{ /*temp*/
		chunk_buf[0][0].pos = (v2i16){0, 0};
		chunk_buf[0][1].pos = (v2i16){0, -1};
		chunk_buf[0][2].pos = (v2i16){0, -2};
		chunk_buf[0][3].pos = (v2i16){-1, 0};
		chunk_buf[0][4].pos = (v2i16){0, 1};
		chunk_buf[0][5].pos = (v2i16){0, 2};
		chunk_buf[0][6].pos = (v2i16){-1, -10};
		chunk_buf[0][7].pos = (v2i16){1, 0};
		parse_chunk_states(&chunk_buf[0][0], 2);
		parse_chunk_states(&chunk_buf[0][1], 1);
		parse_chunk_states(&chunk_buf[0][2], 1);
		parse_chunk_states(&chunk_buf[0][3], 30);
		parse_chunk_states(&chunk_buf[0][4], 2);
		parse_chunk_states(&chunk_buf[0][5], 2);
		parse_chunk_states(&chunk_buf[0][6], 9);
		parse_chunk_states(&chunk_buf[0][7], 2);
	}
	lily.state |= STATE_FALLING; //temp
	lily.state &= ~STATE_PARSE_TARGET;

	if (ModeDebug)
	{
		state |= STATE_DEBUG;
		lily.state |= STATE_FLYING;
	}

	state |= STATE_ACTIVE | STATE_HUD;
}

void main_loop()
{
	start_time = get_time_ms();
	win.scl.x = GetRenderWidth();
	win.scl.y = GetRenderHeight();

	parse_player_states(&lily);
	!ModeCollide ?: give_collision_static(&lily, &target_coordinates_feet);
	give_camera_movements_player(&lily);
	if (state & STATE_DEBUG)
		give_camera_movements_debug_info(&lily);
	(lily.state & STATE_MENU_OPEN || state & STATE_SUPER_DEBUG) ? show_cursor : hide_cursor;
	if (!(lily.state & STATE_MENU_OPEN) && !(state & STATE_SUPER_DEBUG) &&
			((GetMouseDelta().x > 2 || GetMouseDelta().x < -2) ||
			 (GetMouseDelta().y > 2 || GetMouseDelta().y < -2)))
		center_cursor;
	listen(&lily);

	BeginDrawing();
	ClearBackground(COL_SKYBOX); /* TODO: make actual skybox */
	BeginMode3D(lily.camera); /* -------------------------------------------- */
	{ /*temp*/
		draw_chunk(&chunk_buf[0][0], 2);
		draw_chunk(&chunk_buf[0][1], 1);
		draw_chunk(&chunk_buf[0][2], 1);
		draw_chunk(&chunk_buf[0][3], 30);
		draw_chunk(&chunk_buf[0][4], 2);
		draw_chunk(&chunk_buf[0][5], 2);
		draw_chunk(&chunk_buf[0][6], 9);
		draw_chunk(&chunk_buf[0][7], 2);
	}

	if (is_range_within_v3fi(&lily.camera.target, (v3i32){-WORLD_SIZE, -WORLD_SIZE, WORLD_BOTTOM}, (v3i32){WORLD_SIZE, WORLD_SIZE, world_height}))
	{
		if (check_target_delta_position(&lily.camera.target, &lily.previous_target))
		{
			target_chunk = get_chunk(&lily.previous_target, &lily.state, STATE_PARSE_TARGET);
			printf("targetxyz[%d, %d, %d]\t\tstate[%d]\n", lily.previous_target.x, lily.previous_target.y, lily.previous_target.z, target_chunk->i[lily.previous_target.z - WORLD_BOTTOM][lily.previous_target.y][lily.previous_target.x]); /*temp*/
		}
		if (lily.state & STATE_PARSE_TARGET)
			if (target_chunk->i[lily.previous_target.z - WORLD_BOTTOM][lily.previous_target.y][lily.previous_target.x] & NOT_EMPTY)
				draw_block_wires(&lily.previous_target);
	}

	if (ModeDebug)
	{
		/*temp
		  draw_block_wires(&target_coordinates_feet);
		  printf("feet: %d %d %d\n", target_coordinates_feet.x, target_coordinates_feet.y, target_coordinates_feet.z);
		*/
		DrawCubeWiresV(lily.camera.target, (Vector3){1, 1, 1}, GREEN);
		draw_bounding_box(&lily.pos, &lily.scl);

		rlPushMatrix();
		rlBegin(RL_LINES);

		draw_line_3d((v3i32){-4, -4, 0}, (v3i32){4, -4, 0}, WHITE);
		draw_line_3d((v3i32){-4, -3, 0}, (v3i32){4, -3, 0}, WHITE);
		draw_line_3d((v3i32){-4, -2, 0}, (v3i32){4, -2, 0}, WHITE);
		draw_line_3d((v3i32){-4, -1, 0}, (v3i32){4, -1, 0}, WHITE);
		draw_line_3d((v3i32){-4, 0, 0}, (v3i32){4, 0, 0}, WHITE); /*center*/
		draw_line_3d((v3i32){-4, 1, 0}, (v3i32){4, 1, 0}, WHITE);
		draw_line_3d((v3i32){-4, 2, 0}, (v3i32){4, 2, 0}, WHITE);
		draw_line_3d((v3i32){-4, 3, 0}, (v3i32){4, 3, 0}, WHITE);
		draw_line_3d((v3i32){-4, 4, 0}, (v3i32){4, 4, 0}, WHITE);

		draw_line_3d((v3i32){-4, -4, 0}, (v3i32){-4, 4, 0}, RAYWHITE);
		draw_line_3d((v3i32){-3, -4, 0}, (v3i32){-3, 4, 0}, RAYWHITE);
		draw_line_3d((v3i32){-2, -4, 0}, (v3i32){-2, 4, 0}, RAYWHITE);
		draw_line_3d((v3i32){-1, -4, 0}, (v3i32){-1, 4, 0}, RAYWHITE);
		draw_line_3d((v3i32){0, -4, 0}, (v3i32){0, 4, 0}, RAYWHITE); /*center*/
		draw_line_3d((v3i32){1, -4, 0}, (v3i32){1, 4, 0}, RAYWHITE);
		draw_line_3d((v3i32){2, -4, 0}, (v3i32){2, 4, 0}, RAYWHITE);
		draw_line_3d((v3i32){3, -4, 0}, (v3i32){3, 4, 0}, RAYWHITE);
		draw_line_3d((v3i32){4, -4, 0}, (v3i32){4, 4, 0}, RAYWHITE);

		draw_line_3d(v3izero, (v3i32){2, 0, 0}, COL_X);
		draw_line_3d(v3izero, (v3i32){0, 2, 0}, COL_Y);
		draw_line_3d(v3izero, (v3i32){0, 0, 2}, COL_Z);
		rlEnd();
		rlPopMatrix();
	}

	EndMode3D(); /* ---------------------------------------------------------- */

	if (state & STATE_HUD)
	{
		draw_hud();
		if (state & STATE_DEBUG)
			draw_debug_info();
		if (state_menu_depth)
		{
			if (lily.container_state & STATE_INVENTORY)
				draw_inventory_survival();
		}
	}

	if (state & STATE_SUPER_DEBUG)
		draw_super_debugger();
	EndDrawing();

	if (state & STATE_PAUSED)
	{
		BeginDrawing();
		draw_menu_overlay;
		EndDrawing();
		while (state & STATE_PAUSED && state & STATE_ACTIVE)
		{
			BeginDrawing();
			listen_menus(&lily);
			draw_game_menu();
			EndDrawing();
		}
		lily.state &= ~STATE_MENU_OPEN;
	}
}

void main_close()
{
	unload_textures();
	free_gui();
	free_super_debugger();
	CloseWindow();
	if (instance != 0) fclose(instance);
}

int main(int argc, char **argv)
{
	if (argc >= 2 && argv[1])
	{
		if (!strncmp(argv[1], "test", 4)) /*debug mode*/
		{
			test();
			return 0;
		}
		init_instance_dir(&argv[1]);
	}

	main_init();
	test(); /*temp*/
	while (state & STATE_ACTIVE) main_loop();
	main_close();
	return 0;
}

// =============================================================================
// _section_listeners_&_input ==================================================
// =============================================================================

void listen(player *player)
{
	// ---- movement -----------------------------------------------------------
	if (IsKeyPressed(bind_jump))
		get_double_press(player, bind_jump) ? player->state ^= STATE_FLYING : 0;
	if (IsKeyDown(bind_jump))
	{
		if (player->state & STATE_FLYING)
			player->pos.z += player->movement_speed;
		else if (player->state & STATE_CAN_JUMP)
		{
			player->v.z += PLAYER_JUMP_HEIGHT;
			player->state &= ~STATE_CAN_JUMP;
		}
	}

	if (IsKeyDown(bind_sneak))
	{
		player->state & STATE_FLYING ?
			(player->pos.z -= player->movement_speed) :
			(player->state |= STATE_SNEAKING);
	}
	IsKeyUp(bind_sneak) ? player->state &= ~STATE_SNEAKING : 0;

	IsKeyDown(bind_sprint) && IsKeyDown(bind_walk_forwards) ? player->state |= STATE_SPRINTING : 0;
	IsKeyUp(bind_sprint) && IsKeyUp(bind_walk_forwards) ? player->state &= ~STATE_SPRINTING : 0;

	if (IsKeyDown(bind_strafe_left))
	{
		player->pos.x -= player->movement_speed*sinf(player->yaw*DEG2RAD);
		player->pos.y += player->movement_speed*cosf(player->yaw*DEG2RAD);
	}

	if (IsKeyDown(bind_strafe_right))
	{
		player->pos.x += player->movement_speed*sinf(player->yaw*DEG2RAD);
		player->pos.y -= player->movement_speed*cosf(player->yaw*DEG2RAD);
	}

	if (IsKeyDown(bind_walk_backwards))
	{
		player->pos.x -= player->movement_speed*cosf(player->yaw*DEG2RAD);
		player->pos.y -= player->movement_speed*sinf(player->yaw*DEG2RAD);
	}

	if (IsKeyPressed(bind_walk_forwards))
		get_double_press(player, bind_walk_forwards) ? player->state |= STATE_SPRINTING : 0;
	if (IsKeyDown(bind_walk_forwards))
	{
		player->pos.x += player->movement_speed*cosf(player->yaw*DEG2RAD);
		player->pos.y += player->movement_speed*sinf(player->yaw*DEG2RAD);
	}

	// ---- gameplay -----------------------------------------------------------
	if (IsMouseButtonDown(bind_attack_or_destroy))
	{
		if (player->state & STATE_PARSE_TARGET)
		{
			target_chunk->i[lily.previous_target.z - WORLD_BOTTOM][lily.previous_target.y][lily.previous_target.x] = 0;
			parse_block_state(target_chunk, lily.previous_target.x, lily.previous_target.y, floorf(lily.previous_target.z - WORLD_BOTTOM));
		}
		/*old
		//TODO: refine (make correct targeting)
		u32 block_index = convert_coordinates_to_chunk_index(&lily.previous_target);
		chunk_buf[target_chunk].index[block_index] = 0;
		//parse_chunk_states(&chunk_buf[target_chunk]); //temp
		calculate_surrounding_block_state(&chunk_buf[target_chunk], block_index);
		*/
	}

	if (IsMouseButtonDown(bind_pick_block))
	{
	}

	if (IsMouseButtonDown(bind_use_item_or_place_block))
	{
		/*old
		//TODO: refine (make correct targeting)
		u32 block_index = convert_coordinates_to_chunk_index(&lily.previous_target);
		chunk_buf[target_chunk].index[block_index] |= NOT_EMPTY;
		//parse_chunk_states(&chunk_buf[target_chunk]); //temp
		calculate_surrounding_block_state(&chunk_buf[target_chunk], block_index);
		*/
	}

	// ---- inventory ----------------------------------------------------------
	IsKeyPressed(bind_hotbar_slot_1) || IsKeyPressed(bind_hotbar_slot_kp_1) ? hud_hotbar_slot_selected = 1 : 0;
	IsKeyPressed(bind_hotbar_slot_2) || IsKeyPressed(bind_hotbar_slot_kp_2) ? hud_hotbar_slot_selected = 2 : 0;
	IsKeyPressed(bind_hotbar_slot_3) || IsKeyPressed(bind_hotbar_slot_kp_3) ? hud_hotbar_slot_selected = 3 : 0;
	IsKeyPressed(bind_hotbar_slot_4) || IsKeyPressed(bind_hotbar_slot_kp_4) ? hud_hotbar_slot_selected = 4 : 0;
	IsKeyPressed(bind_hotbar_slot_5) || IsKeyPressed(bind_hotbar_slot_kp_5) ? hud_hotbar_slot_selected = 5 : 0;
	IsKeyPressed(bind_hotbar_slot_6) || IsKeyPressed(bind_hotbar_slot_kp_6) ? hud_hotbar_slot_selected = 6 : 0;
	IsKeyPressed(bind_hotbar_slot_7) || IsKeyPressed(bind_hotbar_slot_kp_7) ? hud_hotbar_slot_selected = 7 : 0;
	IsKeyPressed(bind_hotbar_slot_8) || IsKeyPressed(bind_hotbar_slot_kp_8) ? hud_hotbar_slot_selected = 8 : 0;
	IsKeyPressed(bind_hotbar_slot_9) || IsKeyPressed(bind_hotbar_slot_kp_9) ? hud_hotbar_slot_selected = 9 : 0;

	if (IsKeyPressed(bind_open_or_close_inventory))
	{
		if (player->container_state & STATE_INVENTORY && player->state & STATE_MENU_OPEN)
		{
			player->container_state &= ~STATE_INVENTORY;
			state_menu_depth = 0;
			player->state &= ~STATE_MENU_OPEN;
		}
		else if (!(player->container_state & STATE_INVENTORY) && !(state & STATE_MENU_OPEN))
		{
			player->container_state |= STATE_INVENTORY;
			player->state |= STATE_MENU_OPEN;
			state_menu_depth = 1;
		}
		if (!(player->container_state & STATE_INVENTORY) && state_menu_depth)
			--state_menu_depth;
	}

	// ---- miscellaneous ------------------------------------------------------
	IsKeyPressed(bind_toggle_hud) ? state ^= STATE_HUD : 0;
	IsKeyPressed(bind_toggle_debug) ? state ^= STATE_DEBUG : 0;

	//TODO: fix fullscreen
	if (IsKeyPressed(bind_toggle_fullscreen))
	{
		state ^= STATE_FULLSCREEN;
		ToggleBorderlessWindowed();

		state & STATE_FULLSCREEN ?
			(SetWindowPosition(0, 0),
			 SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0))) :

			(SetWindowSize(WIDTH, HEIGHT),
			 SetWindowPosition((GetMonitorWidth(0)/2) - (WIDTH/2), (GetMonitorHeight(0)/2) - (HEIGHT/2)));
	}

	if (IsKeyPressed(bind_toggle_perspective))
		player->perspective < 4 ? (++player->perspective) : (player->perspective = 0);

	if (IsKeyPressed(bind_pause))
	{
		if (!state_menu_depth)
		{
			player->container_state = 0;
			state |= STATE_PAUSED;
			state_menu_depth = 1;
			player->state |= STATE_MENU_OPEN;
			show_cursor;
		}
		else --state_menu_depth;
	}
	if (!state_menu_depth && !(state & STATE_SUPER_DEBUG))
	{
		hide_cursor;
		center_cursor;
	}

	// ---- debug --------------------------------------------------------------
	IsKeyPressed(KEY_TAB) ? state ^= STATE_SUPER_DEBUG : 0;
	IsKeyPressed(bind_quit) ? state &= ~STATE_ACTIVE : 0;
}

void listen_menus(player *player)
{
	if ((IsKeyPressed(bind_pause) || button_state_back_to_game == BUTTON_PRESSED) && state_menu_depth == 1)
	{
		state ^= STATE_PAUSED;
		button_state_back_to_game = BUTTON_LISTENING;
		player->state &= ~STATE_MENU_OPEN;
		state_menu_depth = 0;
	}
	if ((IsKeyPressed(bind_quit) || button_state_save_and_quit_to_title == BUTTON_PRESSED) && state_menu_depth)
		state &= ~STATE_ACTIVE;
}

// =============================================================================
// _section_testing ============================================================
// =============================================================================

void test() /* command: './minecraft test' to run only this function*/
{
	printf("chunk block state: %d\n", chunk_buf[0][3].i[0][0][0]);
	printf("chunk start memory addr: %p\n", &chunk_buf[0][3].i[0][0][0]);
	printf("chunk block memory addr: %p\n", &chunk_buf[0][3].i[0][0][63]);
}
