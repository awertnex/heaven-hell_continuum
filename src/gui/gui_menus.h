#ifndef HHC_GUI_MENUS_H
#define HHC_GUI_MENUS_H

#include "deps/fossil/math/vector.h"
#include "deps/fossil/ui/ui_event.h"

#define ui_button_init(element, click_func) \
    ui_button_init_func((fsl_ui_element*)(element), (click_func))

#define ui_button_update(element, pos_x, pos_y, size_x, size_y, align_x, align_y) \
    ui_button_update_func((fsl_ui_element*)(element), (pos_x), (pos_y), \
            (size_x), (size_y), (align_x), (align_y))

void gui_menus_init(v2i32 render_size);
void ui_button_init_func(fsl_ui_element *element, fsl_ui_event_func click_func);

void ui_button_update_func(fsl_ui_element *element, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 align_x, i32 align_y);

void gui_menu_title_update(v2i32 render_size);
void gui_menu_pause_update(v2i32 render_size);
void gui_menu_title_draw(void);
void gui_menu_pause_draw(void);

#endif /* HHC_GUI_MENUS_H */
