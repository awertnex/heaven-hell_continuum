#include "deps/fossil/input.h"
#include "deps/fossil/math.h"
#include "deps/fossil/shaders.h"
#include "deps/fossil/time.h"

#include "h/assets.h"
#include "h/chunking.h"
#include "h/gui.h"
#include "h/input.h"
#include "h/logger.h"
#include "h/player.h"
#include "h/world.h"

#include <math.h>

/* ---- movement ------------------------------------------------------------ */

u32 bind_walk_forward = {0};
u32 bind_walk_backward = {0};
u32 bind_strafe_left = {0};
u32 bind_strafe_right = {0};
u32 bind_jump = {0};
u32 bind_sprint = {0};
u32 bind_sneak = {0};

/* ---- gameplay ------------------------------------------------------------ */

u32 bind_attack_or_destroy = {0};
u32 bind_sample_block = {0};
u32 bind_build_or_use = {0};

/* ---- inventory ----------------------------------------------------------- */

u32 bind_drop_item = {0};
u32 bind_inventory = {0};
u32 bind_hotbar[2][PLAYER_HOTBAR_SLOTS_MAX] = {0};

/* ---- miscellaneous ------------------------------------------------------- */

u32 bind_toggle_hud = {0};
u32 bind_take_screenshot = {0};
u32 bind_toggle_debug = {0};
u32 bind_toggle_cinematic_camera = {0};
u32 bind_toggle_perspective = {0};
u32 bind_toggle_cinematic_motion = {0};
u32 bind_toggle_fullscreen = {0};
u32 bind_zoom = {0};
u32 bind_toggle_flashlight = {0};
u32 bind_pause = {0};
u32 bind_chat_or_command = {0};
u32 bind_reload_shaders = {0};

/* ---- debug & menu -------------------------------------------------------- */

/* TODO: navigate menus with arrow keys.
 */
u32 bind_left = {0};
u32 bind_right = {0};
u32 bind_down = {0};
u32 bind_up = {0};

u32 bind_debug_mod = FSL_KEY_LEFT_ALT;
u32 bind_toggle_super_debug = {0};
u32 bind_toggle_trans_blocks = {0};
u32 bind_toggle_chunk_bounds = {0};
u32 bind_toggle_bounding_boxes = {0};
u32 bind_toggle_chunk_gizmo = {0};
u32 bind_toggle_chunk_queue_visualizer = {0};

void input_init(void)
{
    /* ---- movement -------------------------------------------------------- */

    bind_walk_forward = FSL_KEY_W;
    bind_walk_backward = FSL_KEY_S;
    bind_strafe_left = FSL_KEY_A;
    bind_strafe_right = FSL_KEY_D;
    bind_jump = FSL_KEY_SPACE;
    bind_sprint = FSL_KEY_LEFT_SHIFT;
    bind_sneak = FSL_KEY_LEFT_CONTROL;

    /* ---- gameplay -------------------------------------------------------- */

    bind_attack_or_destroy = GLFW_MOUSE_BUTTON_LEFT;
    bind_sample_block = GLFW_MOUSE_BUTTON_MIDDLE;
    bind_build_or_use = GLFW_MOUSE_BUTTON_RIGHT;

    /* ---- inventory ------------------------------------------------------- */

    bind_drop_item = FSL_KEY_Q;
    bind_inventory = FSL_KEY_E;

    bind_hotbar[0][1] = FSL_KEY_1;
    bind_hotbar[0][2] = FSL_KEY_2;
    bind_hotbar[0][3] = FSL_KEY_3;
    bind_hotbar[0][4] = FSL_KEY_4;
    bind_hotbar[0][5] = FSL_KEY_5;
    bind_hotbar[0][6] = FSL_KEY_6;
    bind_hotbar[0][7] = FSL_KEY_7;
    bind_hotbar[0][8] = FSL_KEY_8;
    bind_hotbar[0][9] = FSL_KEY_9;
    bind_hotbar[0][0] = FSL_KEY_0;
    bind_hotbar[1][1] = FSL_KEY_KP_1;
    bind_hotbar[1][2] = FSL_KEY_KP_2;
    bind_hotbar[1][3] = FSL_KEY_KP_3;
    bind_hotbar[1][4] = FSL_KEY_KP_4;
    bind_hotbar[1][5] = FSL_KEY_KP_5;
    bind_hotbar[1][6] = FSL_KEY_KP_6;
    bind_hotbar[1][7] = FSL_KEY_KP_7;
    bind_hotbar[1][8] = FSL_KEY_KP_8;
    bind_hotbar[1][9] = FSL_KEY_KP_9;
    bind_hotbar[1][0] = FSL_KEY_KP_0;

    /* ---- miscellaneous --------------------------------------------------- */

    bind_toggle_hud = FSL_KEY_F1;
    bind_take_screenshot = FSL_KEY_F2;
    bind_toggle_debug = FSL_KEY_F3;
    bind_toggle_cinematic_camera = FSL_KEY_F4;
    bind_toggle_perspective = FSL_KEY_F5;
    bind_toggle_cinematic_motion = FSL_KEY_F6;
    bind_toggle_fullscreen = FSL_KEY_F11;
    bind_zoom = FSL_KEY_Z;
    bind_toggle_flashlight = FSL_KEY_F;
    bind_pause = FSL_KEY_ESCAPE;
    bind_chat_or_command = FSL_KEY_SLASH;

    /* ---- debug & menu ---------------------------------------------------- */

    bind_left = FSL_KEY_LEFT;
    bind_right = FSL_KEY_RIGHT;
    bind_down = FSL_KEY_DOWN;
    bind_up = FSL_KEY_UP;
    bind_toggle_super_debug = FSL_KEY_TAB;
    bind_toggle_trans_blocks = FSL_KEY_T;
    bind_toggle_chunk_bounds = FSL_KEY_C;
    bind_toggle_bounding_boxes = FSL_KEY_B;
    bind_toggle_chunk_gizmo = FSL_KEY_G;
    bind_toggle_chunk_queue_visualizer = FSL_KEY_V;
    bind_reload_shaders = FSL_KEY_L;
}

void input_update(player *p)
{
    u32 i = 0;
    f32 px = 0.0f, nx = 0.0f;
    f32 py = 0.0f, ny = 0.0f;
    f32 pz = 0.0f, nz = 0.0f;
    f32 spch = sin(p->pitch * FSL_DEG2RAD);
    f32 cpch = cos(p->pitch * FSL_DEG2RAD);
    f32 syaw = sin(p->yaw * FSL_DEG2RAD);
    f32 cyaw = cos(p->yaw * FSL_DEG2RAD);

    p->input.x = 0.0;
    p->input.y = 0.0;
    p->input.z = 0.0;

    if (!(p->flag & FLAG_PLAYER_DEAD))
    {
        /* ---- movement ---------------------------------------------------- */

        px += (f32)fsl_is_key_hold(bind_walk_forward);
        nx += (f32)fsl_is_key_hold(bind_walk_backward);
        py += (f32)fsl_is_key_hold(bind_strafe_left);
        ny += (f32)fsl_is_key_hold(bind_strafe_right);

        if (fsl_is_key_press_double(bind_walk_forward))
            p->flag |= FLAG_PLAYER_SPRINTING;

        /* ---- jumping ----------------------------------------------------- */

        if (fsl_is_key_hold(bind_jump))
        {
            if (p->flag & FLAG_PLAYER_FLYING)
                pz += 1.0f;
            else if (p->flag & FLAG_PLAYER_CAN_JUMP)
            {
                p->velocity.z += sqrtf(2.0f * world.gravity * PLAYER_JUMP_HEIGHT);
                p->flag &= ~FLAG_PLAYER_CAN_JUMP;
            }
        }

        if (fsl_is_key_press_double(bind_jump))
            p->flag ^= FLAG_PLAYER_FLYING;

        /* ---- sprinting --------------------------------------------------- */

        if (fsl_is_key_hold(bind_sprint) && fsl_is_key_hold(bind_walk_forward))
            p->flag |= FLAG_PLAYER_SPRINTING;
        else if (fsl_is_key_release(bind_walk_forward))
            p->flag &= ~FLAG_PLAYER_SPRINTING;

        /* ---- sneaking ---------------------------------------------------- */

        if (fsl_is_key_hold(bind_sneak))
        {
            if (p->flag & FLAG_PLAYER_FLYING)
                nz += 1.0f;
            else p->flag |= FLAG_PLAYER_SNEAKING;
        }
        else p->flag &= ~FLAG_PLAYER_SNEAKING;

        /* ---- apply input ------------------------------------------------- */

        if (p->flag & FLAG_PLAYER_FLYING && p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        {
            p->input.x =
                (px - nx) * cyaw * cpch +
                (py - ny) * -cos(p->yaw * FSL_DEG2RAD + FSL_PI / 2.0) +
                (pz - nz) * cyaw * spch;
            p->input.y =
                (px - nx) * -syaw * cpch +
                (py - ny) * sin(p->yaw * FSL_DEG2RAD + FSL_PI / 2.0) +
                (pz - nz) * -syaw * spch;
            p->input.z =
                (px - nx) * -spch +
                (pz - nz) * cpch;
        }
        else
        {
            p->input.x =
                (px - nx) * cyaw +
                (py - ny) * -cos(p->yaw * FSL_DEG2RAD + FSL_PI / 2.0);
            p->input.y =
                (px - nx) * -syaw +
                (py - ny) * sin(p->yaw * FSL_DEG2RAD + FSL_PI / 2.0);
            p->input.z =
                pz - nz;
        }

        p->input = fsl_normalize_v3f32(p->input);

        /* ---- gameplay ---------------------------------------------------- */

        if (
                !core.flag.chunk_buf_dirty &&
                core.flag.parse_target &&
                chunk_tab[chunk_tab_index])
        {
            if (fsl_is_mouse_hold(bind_attack_or_destroy))
            {
                block_break(chunk_tab_index,
                        (i64)p->target.x - chunk_tab[chunk_tab_index]->pos.x * CHUNK_DIAMETER,
                        (i64)p->target.y - chunk_tab[chunk_tab_index]->pos.y * CHUNK_DIAMETER,
                        (i64)p->target.z - chunk_tab[chunk_tab_index]->pos.z * CHUNK_DIAMETER);
            }
            if (fsl_is_mouse_press(bind_build_or_use))
            {
                block_place(chunk_tab_index,
                        (i64)p->target.x - chunk_tab[chunk_tab_index]->pos.x * CHUNK_DIAMETER,
                        (i64)p->target.y - chunk_tab[chunk_tab_index]->pos.y * CHUNK_DIAMETER,
                        (i64)p->target.z - chunk_tab[chunk_tab_index]->pos.z * CHUNK_DIAMETER,
                        p->target_normal, p->hotbar_slots[p->hotbar_slot_selected]);
            }

            if (fsl_is_key_press(bind_sample_block)) {}
        }

        /* ---- inventory --------------------------------------------------- */

        for (i = 0; i < PLAYER_HOTBAR_SLOTS_MAX; ++i)
            if (fsl_is_key_press(bind_hotbar[0][i]) || fsl_is_key_press(bind_hotbar[1][i]))
                p->hotbar_slot_selected = i;

        if (fsl_is_key_press(bind_inventory))
        {
            if ((p->menu_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && state_menu_depth)
            {
                state_menu_depth = 0;
                p->menu_state &= ~STATE_PLAYER_MENU_INVENTORY_SURVIVAL;
            }
            else if (!(p->menu_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && !state_menu_depth)
            {
                state_menu_depth = 1;
                p->menu_state |= STATE_PLAYER_MENU_INVENTORY_SURVIVAL;
            }

            if (!(p->menu_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && state_menu_depth)
                --state_menu_depth;
        }

        /* ---- miscellaneous ----------------------------------------------- */

        if (fsl_is_key_press(bind_toggle_hud))
            core.flag.hud ^= 1;
        if (fsl_is_key_press(bind_take_screenshot))
            fsl_request_screenshot();

        if (fsl_is_key_press(bind_toggle_debug))
            core.flag.debug ^= 1;

        if (fsl_is_key_press(bind_toggle_perspective))
            p->camera_mode = (p->camera_mode + 1) % PLAYER_CAMERA_MODE_COUNT;

        if (fsl_is_key_press(bind_zoom))
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "Zoom Toggled On"));
        if (fsl_is_key_hold(bind_zoom))
            p->flag |= FLAG_PLAYER_ZOOMER;
        if (fsl_is_key_release(bind_zoom))
        {
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "Zoom Toggled Off"));
            p->flag &= ~FLAG_PLAYER_ZOOMER;
        }

        if (fsl_is_key_press(bind_toggle_flashlight))
        {
            p->flag ^= FLAG_PLAYER_FLASHLIGHT;

            if (p->flag & FLAG_PLAYER_FLASHLIGHT)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Flashlight Toggled On"));
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Flashlight Toggled Off"));
        }

        if (fsl_is_key_press(bind_toggle_cinematic_motion))
        {
            p->flag ^= FLAG_PLAYER_CINEMATIC_MOTION;

            if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Cinematic Motion On"));
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Cinematic Motion Off"));
        }
    }

    /* ---- debug ----------------------------------------------------------- */

#if !HHC_RELEASE_BUILD
    if (fsl_is_key_press(bind_toggle_super_debug))
        core.flag.super_debug ^= 1;
#endif /* HHC_RELEASE_BUILD */

    if (fsl_is_key_hold(bind_debug_mod))
    {
        if (fsl_is_key_press(bind_toggle_trans_blocks))
        {
            core.debug.trans_blocks ^= 1;

            if (core.debug.trans_blocks)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "View Transparent Blocks On"));
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "View Transparent Blocks Off"));
        }

        if (fsl_is_key_press(bind_toggle_chunk_bounds))
        {
            core.debug.chunk_bounds ^= 1;

            if (core.debug.chunk_bounds)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "View Chunk Boundaries On"));
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "View Chunk Boundaries Off"));
        }

        if (fsl_is_key_press(bind_toggle_bounding_boxes))
        {
            core.debug.bounding_boxes ^= 1;

            if (core.debug.bounding_boxes)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "View Bounding Boxes On"));
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "View Bounding Boxes Off"));
        }

        if (fsl_is_key_press(bind_toggle_chunk_gizmo))
            core.debug.chunk_gizmo ^= 1;

        if (fsl_is_key_press(bind_toggle_chunk_queue_visualizer))
        {
            core.debug.chunk_queue_visualizer ^= 1;

            if (core.debug.chunk_queue_visualizer)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "View Chunk Queue Visualizer On"));
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "View Chunk Queue Visualizer Off"));
        }

        if (fsl_is_key_press(bind_reload_shaders))
        {
            if (fsl_shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_SKYBOX]) == FSL_ERR_SUCCESS)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Shaders Reloaded!"));
        }
    }
}
