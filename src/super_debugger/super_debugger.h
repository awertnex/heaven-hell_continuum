#ifndef HHC_SUPER_DEBUGGER_H
#define HHC_SUPER_DEBUGGER_H

#include "deps/fossil/math/vector.h"
#include "deps/fossil/ui/ui_element.h"

#define SDB_BUTTON_SPACING 3

#define super_debugger_button_init(button, parent, click_func) \
    super_debugger_button_init_func((fsl_ui_element*)(button), (fsl_ui_element*)(parent), (click_func))
#define super_debugger_button_update(button, row) \
    super_debugger_button_update_func((fsl_ui_element*)(button), (row))

enum ui_element_panel_sdb_index
{
    UI_ELEMENT_SDB_PANEL_CONFIG,
    UI_ELEMENT_SDB_PANEL_LOGGER,
    UI_ELEMENT_SDB_PANEL_COUNT
}; /* ui_element_panel_sdb_index */

enum ui_element_button_sdb_index
{
    UI_ELEMENT_SDB_TOGGLE_LOGGER,
    UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS,
    UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES,
    UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS,
    UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO,
    UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER,
    UI_ELEMENT_SDB_TOGGLE_ANTI_ALIASING,
    UI_ELEMENT_SDB_TOGGLE_SCREEN_SPACE_AMBIENT_OCCLUSION,
    UI_ELEMENT_SDB_BUTTON_COUNT
}; /* ui_element_button_sdb_index */

extern fsl_ui_element_panel_9_slice ui_element_sdb_panel[UI_ELEMENT_SDB_PANEL_COUNT];
extern fsl_ui_element_button ui_element_sdb_button[UI_ELEMENT_SDB_BUTTON_COUNT];

void super_debugger_init(v2i32 render_size);

/**
 *  @brief make a button for the main super-debugger panel.
 *
 *  @note called indirectly via a macro-wrapper.
 */
void super_debugger_button_init_func(fsl_ui_element *button, fsl_ui_element *parent,
        fsl_ui_event_func click_func);

/**
 *  @brief update a button for the main super-debugger panel.
 *
 *  @note called indirectly via a macro-wrapper.
 *
 *  @param row button's vertical positioning relative to its parent, based on an
 *  internal alignment size.
 */
void super_debugger_button_update_func(fsl_ui_element *button, i32 row);

void super_debugger_update(v2i32 render_size);
void super_debugger_draw(v2i32 render_size, GLuint fbo);
void super_debugger_panel_config_height_set(b8 logger_panel_is_visible);
void super_debugger_logger_scroll(i32 delta);
void super_debugger_gui_scale_set(f32 scale);

#endif /* HHC_SUPER_DEBUGGER_H */
