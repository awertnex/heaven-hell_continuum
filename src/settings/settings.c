#include "deps/fossil/common/diagnostics.h"
#include "deps/fossil/common/config.h"
#include "deps/fossil/assets/asset_types.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/ui/ui.h"

#include "deps/fossil/h/dir.h"

#include "../h/common.h"
#include "../h/assets.h"
#include "../h/diagnostics.h"
#include "../h/gui.h"
#include "../h/main.h"
#include "../h/player.h"

#include "settings.h"

#include <stdio.h>
#include <string.h>

#define SETTINGS_FILE_SIZE_MAX (8 * 1024)

hhc_settings settings = {0};

u32 settings_init(void)
{
    str tokens[4][24] =
    {
        "mouse_sensitivity",
        "field_of_view",
        "render_distance",
        "target_fps",
    };
    str *file_contents_out = NULL;
    str *file_contents_in = NULL;

    if (fsl_mem_alloc((void*)&file_contents_out, SETTINGS_FILE_SIZE_MAX,
                "settings_init().file_contents_out") != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    if (fsl_is_dir_exists(GAME_DIR_NAME_CONFIG, TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    snprintf(file_contents_out, SETTINGS_FILE_SIZE_MAX,
            "%s = %d\n"
            "%s = %d\n"
            "%s = %d\n"
            "%s = %d\n",
            tokens[0], SET_MOUSE_SENSITIVITY_DEFAULT,
            tokens[1], SET_FOV_DEFAULT,
            tokens[2], SET_RENDER_DISTANCE_DEFAULT,
            tokens[3], FSL_TARGET_FPS_DEFAULT);

    if (fsl_is_file_exists(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS, FALSE) != FSL_ERR_SUCCESS)
    {
        fsl_write_file(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS,
                strlen(file_contents_out),
                file_contents_out, TRUE, TRUE);
    }

    fsl_get_file_contents(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS,
            (void*)&file_contents_in, TRUE);
    if (*GAME_ERR != FSL_ERR_SUCCESS)
        goto cleanup;

    settings.lerp_speed = SET_LERP_SPEED_DEFAULT;

    settings.render_distance = 8;
    settings.chunk_buf_radius = settings.render_distance;
    settings.chunk_buf_diameter = settings.chunk_buf_radius * 2 + 1;

    settings.chunk_buf_layer =
        settings.chunk_buf_diameter *
        settings.chunk_buf_diameter;

    settings.chunk_buf_volume =
        settings.chunk_buf_diameter *
        settings.chunk_buf_diameter *
        settings.chunk_buf_diameter;

    settings.chunk_tab_center =
        settings.chunk_buf_radius +
        settings.chunk_buf_radius * settings.chunk_buf_diameter +
        settings.chunk_buf_radius * settings.chunk_buf_layer;

    settings.reach_distance = PLAYER_REACH_DISTANCE_MAX;
    settings.mouse_sensitivity = SET_MOUSE_SENSITIVITY_DEFAULT * 0.004f;
    settings.font_size = 20.0f;
    settings.target_fps = 0;
    settings.fov = SET_FOV_DEFAULT;
    settings.anti_aliasing = TRUE;

    settings_gui_scale_set(SET_GUI_SCALE_3);

    fsl_mem_free((void*)&file_contents_out, SETTINGS_FILE_SIZE_MAX,
            "settings_init().file_contents_out");
    if (file_contents_in)
        fsl_mem_free((void*)&file_contents_in, strlen(file_contents_in),
                "settings_init().file_contents_in");

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_mem_free((void*)&file_contents_out, SETTINGS_FILE_SIZE_MAX,
            "settings_init().file_contents_out");
    if (file_contents_in)
        fsl_mem_free((void*)&file_contents_in, strlen(file_contents_in),
                "settings_init().file_contents_in");
    return *GAME_ERR;
}

void settings_update(hhc_player *p)
{
    fsl_texture *texture_p = fsl_mem_handle_get(texture);

    /* element: crosshair */
    fsl_ui_element_set_texture(&ui_element[UI_ELEMENT_CROSSHAIR], &texture_p[TEXTURE_CROSSHAIR]);
    fsl_ui_element_set_uv(&ui_element[UI_ELEMENT_CROSSHAIR], 0, 0, 16, 16);
    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_CROSSHAIR],
            render->size.x / 2, render->size.y / 2, 0, 0, 0, 0);
    fsl_ui_element_set_size(&ui_element[UI_ELEMENT_CROSSHAIR], 0, 0, 8, 8);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_CROSSHAIR],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_alignment(&ui_element[UI_ELEMENT_CROSSHAIR], 0, 0);

    /* element: hotbar */
    fsl_ui_element_set_texture(&ui_element[UI_ELEMENT_HOTBAR], &texture_p[TEXTURE_HOTBAR]);
    fsl_ui_element_set_uv(&ui_element[UI_ELEMENT_HOTBAR], 0, 0, 169, 16);
    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_HOTBAR],
            render->size.x / 2, render->size.y, 0, 0, 0, -4);
    fsl_ui_element_set_size(&ui_element[UI_ELEMENT_HOTBAR], 0, 0,
            ui_element[UI_ELEMENT_HOTBAR].texture->size.x,
            ui_element[UI_ELEMENT_HOTBAR].texture->size.y);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_HOTBAR],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_alignment(&ui_element[UI_ELEMENT_HOTBAR], 0, 1);

    /* element: hotbar selected */
    fsl_ui_element_set_texture(&ui_element[UI_ELEMENT_HOTBAR_SELECTED],
            &texture_p[TEXTURE_HOTBAR]);
    fsl_ui_element_set_uv(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], 0, 16, 18, 18);
    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_HOTBAR_SELECTED],
            render->size.x / 2, render->size.y, 0, 0,
            (-169 / 2) - 1 + p->hotbar_slot_selected * 17, -3);
    fsl_ui_element_set_size(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], 0, 0,
            ui_element[UI_ELEMENT_HOTBAR_SELECTED].texture->size.x,
            ui_element[UI_ELEMENT_HOTBAR_SELECTED].texture->size.y);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_HOTBAR_SELECTED],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_alignment(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], -1, 1);

    /* element: container inventory survival */
    fsl_ui_element_set_texture(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL],
            &texture_p[TEXTURE_CONTAINER_INVENTORY_SURVIVAL]);
    fsl_ui_element_set_uv(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL], 0, 0, 177, 177);
    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL],
            render->size.x / 2, render->size.y / 2, 0, 0, 0, 0);
    fsl_ui_element_set_size(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL], 0, 0,
            ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL].texture->size.x,
            ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL].texture->size.y);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL],
            settings.gui_scale, settings.gui_scale);
}

void settings_gui_scale_set(f32 scale)
{
    settings.gui_scale = scale;

    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_CROSSHAIR],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_HOTBAR],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_HOTBAR_SELECTED],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL],
            settings.gui_scale, settings.gui_scale);
}
