#include "h/super_debugger.h"
#include "h/gui.h"
#include "h/logic.h"

// ---- variables --------------------------------------------------------------
Texture texture_super_debugger;
DebugRect debug_rectangle =
{
    .corner_00 =    {0,                 0,                  SDB_BASE_SIZE,  SDB_BASE_SIZE},
    .corner_10 =    {SDB_BASE_SIZE,     0,                  SDB_BASE_SIZE,  SDB_BASE_SIZE},
    .corner_01 =    {0,                 SDB_BASE_SIZE,      SDB_BASE_SIZE,  SDB_BASE_SIZE},
    .corner_11 =    {SDB_BASE_SIZE,     SDB_BASE_SIZE,      SDB_BASE_SIZE,  SDB_BASE_SIZE},
    .edge_left =    {0,                 SDB_BASE_SIZE,      SDB_BASE_SIZE,  1},
    .edge_right =   {SDB_BASE_SIZE,     SDB_BASE_SIZE,      SDB_BASE_SIZE,  1},
    .edge_top =     {SDB_BASE_SIZE,     0,                  1,              SDB_BASE_SIZE},
    .edge_bottom =  {SDB_BASE_SIZE,     SDB_BASE_SIZE,      1,              SDB_BASE_SIZE},
    .rect_center =  {SDB_BASE_SIZE - 1, SDB_BASE_SIZE - 1,  2,              2},
};

Rectangle debug_button_add = {SDB_BASE_SIZE*2,                      0, SDB_BUTTON_SIZE, SDB_BUTTON_SIZE};
Rectangle debug_button_sub = {(SDB_BASE_SIZE*2) + SDB_BUTTON_SIZE,  0, SDB_BUTTON_SIZE, SDB_BUTTON_SIZE};

// ---- functions --------------------------------------------------------------
void init_super_debugger(v2f32 render_size)
{
    buttons[BTN_SDB_ADD] = 1;
    buttons[BTN_SDB_SUB] = 1;

    texture_super_debugger = LoadTexture("resources/gui/containers/super_debugger.png");
    debug_rectangle.scl.x = 300;
    debug_rectangle.scl.y = render_size.y - ((MARGIN + SDB_BASE_SIZE)*2);
    debug_rectangle.pos.x = render_size.x - debug_rectangle.scl.x - MARGIN - (SDB_BASE_SIZE*2);
    debug_rectangle.pos.y = MARGIN + SDB_BASE_SIZE;
}

void free_super_debugger()
{
    buttons[BTN_SDB_ADD] = 0;
    buttons[BTN_SDB_SUB] = 0;

    UnloadTexture(texture_super_debugger);
}

void draw_super_debugger(v2f32 render_size)
{
    debug_rectangle.scl.y = render_size.y - ((MARGIN + SDB_BASE_SIZE)*2);
    debug_rectangle.pos.x = render_size.x - debug_rectangle.scl.x - MARGIN - (SDB_BASE_SIZE*2);

    // ---- draw base ----------------------------------------------------------
    draw_texture_simple(texture_super_debugger, debug_rectangle.rect_center,
            (v2i16){debug_rectangle.pos.x, debug_rectangle.pos.y},
            (v2i16){debug_rectangle.scl.x, debug_rectangle.scl.y},
            COL_TRANS_MENU);
    
    // ---- draw edges ---------------------------------------------------------
    draw_texture_simple(texture_super_debugger, debug_rectangle.edge_left,
            (v2i16){debug_rectangle.pos.x - SDB_BASE_SIZE, debug_rectangle.pos.y},
            (v2i16){SDB_BASE_SIZE, debug_rectangle.scl.y},
            COL_TRANS_MENU);

    draw_texture_simple(texture_super_debugger, debug_rectangle.edge_right,
            (v2i16){debug_rectangle.pos.x + debug_rectangle.scl.x, debug_rectangle.pos.y},
            (v2i16){SDB_BASE_SIZE, debug_rectangle.scl.y},
            COL_TRANS_MENU);

    draw_texture_simple(texture_super_debugger, debug_rectangle.edge_top,
            (v2i16){debug_rectangle.pos.x, debug_rectangle.pos.y - SDB_BASE_SIZE},
            (v2i16){debug_rectangle.scl.x, SDB_BASE_SIZE},
            COL_TRANS_MENU);

    draw_texture_simple(texture_super_debugger, debug_rectangle.edge_bottom,
            (v2i16){debug_rectangle.pos.x, debug_rectangle.pos.y + debug_rectangle.scl.y},
            (v2i16){debug_rectangle.scl.x, SDB_BASE_SIZE},
            COL_TRANS_MENU);

    // ---- draw corners -------------------------------------------------------
    draw_texture(texture_super_debugger, debug_rectangle.corner_00,
            (v2i16){debug_rectangle.pos.x - SDB_BASE_SIZE, debug_rectangle.pos.y - SDB_BASE_SIZE},
            (v2i16){1, 1},
            0, 0, COL_TRANS_MENU);

    draw_texture(texture_super_debugger, debug_rectangle.corner_10,
            (v2i16){debug_rectangle.pos.x + debug_rectangle.scl.x, debug_rectangle.pos.y - SDB_BASE_SIZE},
            (v2i16){1, 1},
            0, 0, COL_TRANS_MENU);

    draw_texture(texture_super_debugger, debug_rectangle.corner_01,
            (v2i16){debug_rectangle.pos.x - SDB_BASE_SIZE, debug_rectangle.pos.y + debug_rectangle.scl.y},
            (v2i16){1, 1},
            0, 0, COL_TRANS_MENU);

    draw_texture(texture_super_debugger, debug_rectangle.corner_11,
            (v2i16){debug_rectangle.pos.x + debug_rectangle.scl.x, debug_rectangle.pos.y + debug_rectangle.scl.y},
            (v2i16){1, 1},
            0, 0, COL_TRANS_MENU);

    // ---- draw the rest of the stuff -----------------------------------------
    /* temp
    draw_button(texture_super_debugger, debug_button_add,
            (v2i16){
            debug_rectangle.pos.x + ((debug_rectangle.scl.x/3)*2),
            debug_rectangle.pos.y + (SDB_BASE_SIZE*2)}, BTN_SDB_ADD, 0);

    draw_button(texture_super_debugger, debug_button_sub,
            (v2i16){
            debug_rectangle.pos.x + ((debug_rectangle.scl.x/3)*2) + (debug_button_add.width*2),
            debug_rectangle.pos.y + (SDB_BASE_SIZE*2)}, BTN_SDB_SUB, 0);
            */
}

