#include "deps/fossil/common/config.h"
#include "deps/fossil/assets/assets.h"
#include "deps/fossil/engine/engine_assets.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/string/string.h"
#include "deps/fossil/ui/ui.h"

#include "../settings/settings.h"

#include "../h/assets.h"
#include "../h/main.h"

#include "super_debugger.h"
#include "super_debugger_callbacks.h"

fsl_ui_element_panel_9_slice ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_COUNT] = {0};
fsl_ui_element_button ui_element_sdb_button[UI_ELEMENT_SDB_BUTTON_COUNT] = {0};

static i32 logger_scroll_pos = 0;

void super_debugger_init(v2i32 render_size)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);

    fsl_ui_element_type_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            FSL_UI_ELEMENT_TYPE_PANEL_9_SLICE);
    fsl_ui_element_type_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER],
            FSL_UI_ELEMENT_TYPE_PANEL_9_SLICE);
    fsl_ui_element_texture_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            &fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_INACTIVE]);
    fsl_ui_element_texture_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER],
            &fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_INACTIVE]);
    fsl_ui_element_uv_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG], 0, 0, 16, 16);
    fsl_ui_element_uv_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER], 0, 0, 16, 16);
    fsl_ui_element_9_slice_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG], TRUE, 8);
    fsl_ui_element_9_slice_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER], TRUE, 8);
    fsl_ui_element_callback_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_panel_enter_func, NULL);
    fsl_ui_element_callback_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_panel_enter_func, NULL);
    fsl_ui_element_callback_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_panel_leave_func, NULL);
    fsl_ui_element_callback_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_panel_leave_func, NULL);

    super_debugger_button_init(&ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_LOGGER],
            &ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            super_debugger_button_click_func_toggle_logger);
    super_debugger_button_init(&ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            &ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            super_debugger_button_click_func_toggle_trans_blocks);
    super_debugger_button_init(&ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            &ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            super_debugger_button_click_func_toggle_bounding_boxes);
    super_debugger_button_init(&ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            &ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            super_debugger_button_click_func_toggle_chunk_bounds);
    super_debugger_button_init(&ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            &ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            super_debugger_button_click_func_toggle_chunk_gizmo);
    super_debugger_button_init(&ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            &ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            super_debugger_button_click_func_toggle_chunk_scheduler_visualizer);
    super_debugger_button_init(&ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_ANTI_ALIASING],
            &ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            super_debugger_button_click_func_toggle_anti_aliasing);
    super_debugger_button_init(&ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_SCREEN_SPACE_AMBIENT_OCCLUSION],
            &ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            super_debugger_button_click_func_toggle_screen_space_ambient_occlusion);

    super_debugger_update(render_size);
}

void super_debugger_button_init_func(fsl_ui_element *button, fsl_ui_element *parent,
        fsl_ui_event_func click_func)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);

    fsl_ui_element_type_set(button, FSL_UI_ELEMENT_TYPE_BUTTON);
    fsl_ui_element_texture_set(button, &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE]);
    fsl_ui_element_uv_set(button, 0, 0, 16, 16);
    fsl_ui_element_9_slice_set(button, FALSE, 8);
    fsl_ui_element_callback_set(button, FSL_UI_EVENT_TYPE_ENTER, super_debugger_button_enter_func, NULL);
    fsl_ui_element_callback_set(button, FSL_UI_EVENT_TYPE_LEAVE, super_debugger_button_leave_func, NULL);
    fsl_ui_element_callback_set(button, FSL_UI_EVENT_TYPE_CLICK, click_func, NULL);
    fsl_ui_element_callback_set(button, FSL_UI_EVENT_TYPE_RELEASE, super_debugger_button_release_func, NULL);
    fsl_ui_element_attach(parent, button);
}

void super_debugger_button_update_func(fsl_ui_element *button, i32 row)
{
    fsl_ui_element_position_set(button, button->parent->transform.baked.size.x,
            0, 0, 0, -SET_MARGIN, SET_MARGIN);
    fsl_ui_element_size_set(button, 0, 0, 32, 16);
    fsl_ui_element_alignment_set(button, 1, -(row * SDB_BUTTON_SPACING) - 1);
    fsl_ui_element_bake(button);
}

void super_debugger_update(v2i32 render_size)
{
    i32 i = 0;

    fsl_ui_element_position_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            0, 0, SET_MARGIN, SET_MARGIN, 0, 0);
    fsl_ui_element_position_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER],
            0, render_size.y, SET_MARGIN, -SET_MARGIN, 0, 0);
    super_debugger_panel_config_height_set(core.debug.logger);
    fsl_ui_element_size_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER],
            render_size.x - SET_MARGIN * 2, 400, 0, 0);

    super_debugger_gui_scale_set(settings.gui_scale);

    fsl_ui_element_alignment_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG], -1, -1);
    fsl_ui_element_alignment_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER], -1, 1);

    fsl_ui_element_bake(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG]);
    fsl_ui_element_bake(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER]);

    for (i = 0; i < UI_ELEMENT_SDB_BUTTON_COUNT; ++i)
        super_debugger_button_update(&ui_element_sdb_button[i], i);
}

void super_debugger_draw(v2i32 render_size, GLuint fbo)
{
    u32 color_setting_on = COLOR_TEXT_ENABLED;
    i32 logger_panel_height =
        ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER].header.transform.baked.size.y - SET_MARGIN * 2;
    fsl_log_entry *log_entry = NULL;
    u32 index = 0;
    i32 i = 0;

    /* ---- configs panel --------------------------------------------------- */

    fsl_ui_start(FALSE);

    fsl_ui_element_draw(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG]);

    for (i = 0; i < UI_ELEMENT_SDB_BUTTON_COUNT; ++i)
        fsl_ui_element_draw(&ui_element_sdb_button[i]);

    fsl_text_start(font[FONT_MONO], settings.font_size, 0, FALSE);

    fsl_text_push("Toggle Logger\r",
            SET_MARGIN * 3,
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_LOGGER].header.transform.baked.pos.y +
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_LOGGER].header.transform.baked.size.y / 2.0f,
            0, FSL_TEXT_ALIGN_CENTER, 0, core.debug.logger ? color_setting_on : COLOR_TEXT_DEFAULT);
    fsl_text_push("Toggle Trans Blocks\r",
            SET_MARGIN * 3,
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS].header.transform.baked.pos.y +
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS].header.transform.baked.size.y / 2.0f,
            0, FSL_TEXT_ALIGN_CENTER, 0, core.debug.trans_blocks ? color_setting_on : COLOR_TEXT_DEFAULT);
    fsl_text_push("Toggle Bounding Boxes\r",
            SET_MARGIN * 3,
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES].header.transform.baked.pos.y +
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES].header.transform.baked.size.y / 2.0f,
            0, FSL_TEXT_ALIGN_CENTER, 0, core.debug.bounding_boxes ? color_setting_on : COLOR_TEXT_DEFAULT);
    fsl_text_push("Toggle Chunk Boundaries\r",
            SET_MARGIN * 3,
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS].header.transform.baked.pos.y +
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS].header.transform.baked.size.y / 2.0f,
            0, FSL_TEXT_ALIGN_CENTER, 0, core.debug.chunk_bounds ? color_setting_on : COLOR_TEXT_DEFAULT);
    fsl_text_push("Toggle Chunk Gizmo\r",
            SET_MARGIN * 3,
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO].header.transform.baked.pos.y +
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO].header.transform.baked.size.y / 2.0f,
            0, FSL_TEXT_ALIGN_CENTER, 0, core.debug.chunk_gizmo ? color_setting_on : COLOR_TEXT_DEFAULT);
    fsl_text_push("Toggle Chunk Scheduler Visualizer\r",
            SET_MARGIN * 3,
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER].header.transform.baked.pos.y +
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER].header.transform.baked.size.y / 2.0f,
            0, FSL_TEXT_ALIGN_CENTER, 0, core.debug.chunk_scheduler_visualizer ? color_setting_on : COLOR_TEXT_DEFAULT);
    fsl_text_push("Toggle MSAA\r",
            SET_MARGIN * 3,
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_ANTI_ALIASING].header.transform.baked.pos.y +
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_ANTI_ALIASING].header.transform.baked.size.y / 2.0f,
            0, FSL_TEXT_ALIGN_CENTER, 0, settings.anti_aliasing ? color_setting_on : COLOR_TEXT_DEFAULT);
    fsl_text_push("Toggle SSAO\r",
            SET_MARGIN * 3,
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_SCREEN_SPACE_AMBIENT_OCCLUSION].header.transform.baked.pos.y +
            ui_element_sdb_button[UI_ELEMENT_SDB_TOGGLE_SCREEN_SPACE_AMBIENT_OCCLUSION].header.transform.baked.size.y / 2.0f,
            0, FSL_TEXT_ALIGN_CENTER, 0, settings.screen_space_ambient_occlusion ? color_setting_on : COLOR_TEXT_DEFAULT);

    fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);

    /* ---- logger panel ---------------------------------------------------- */

    if (core.debug.logger)
    {
        fsl_ui_start(FALSE);

        fsl_ui_element_draw(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER]);
        fsl_text_start(font[FONT_MONO_BOLD], settings.font_size, 0, FALSE);

        log_entry = fsl_mem_handle_get(fsl_log_core.buf);
        for (i = logger_panel_height / settings.font_size; i > 0; --i)
        {
            index = fsl_mod_i32(fsl_log_core.cursor - i - logger_scroll_pos, FSL_LOGGER_HISTORY_MAX);
            fsl_text_push(fsl_stringf("%s\n", log_entry[index].message),
                    SET_MARGIN * 2, render_size.y - SET_MARGIN * 2,
                    0, 0, render_size.x - SET_MARGIN * 4,
                    log_entry[index].color);

            if ((i32)fsl_text_height_get() + SET_MARGIN * 2 >= logger_panel_height)
                break;
        }

        /* this "useless" function call aligns all the pushed strings correctly once
         * at the end of the loop, do not touch it. */
        fsl_text_push("", 0, 0, 0, FSL_TEXT_ALIGN_BOTTOM, 0, 0);
        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
    }

    fsl_fbo_blit(fbo);
    fsl_ui_stop();
}

void super_debugger_panel_config_height_set(b8 logger_panel_is_visible)
{
    i32 height = render->size.y - SET_MARGIN * 2;

    if (logger_panel_is_visible)
    {
        height -=
            ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER].header.transform.baked.size.y +
            SET_MARGIN;
        height = fsl_clamp_i32(height,
                ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_LOGGER].quad.slice_size * 4,
                render->size.y);
    }

    fsl_ui_element_size_set(&ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_CONFIG],
            470, height, 0, 0);
}

void super_debugger_gui_scale_set(f32 scale)
{
    u32 i = 0;

    for (i = 0; i < UI_ELEMENT_SDB_PANEL_COUNT; ++i)
        fsl_ui_element_scale_set(&ui_element_sdb_panel[i], scale, scale);

    for (i = 0; i < UI_ELEMENT_SDB_BUTTON_COUNT; ++i)
        fsl_ui_element_scale_set(&ui_element_sdb_button[i], scale, scale);
}

void super_debugger_logger_scroll(i32 delta)
{
    logger_scroll_pos =
        fsl_clamp_i32(logger_scroll_pos + delta * SET_CONSOLE_SCROLL_SPEED, 0, fsl_log_core.cursor);
}
