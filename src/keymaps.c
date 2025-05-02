#pragma once

#include "dependencies/raylib-5.5/src/raylib.h"

// ---- movement ---------------------------------------------------------------
KeyboardKey bindJump =                      KEY_SPACE;
KeyboardKey bindSneak =                     KEY_LEFT_CONTROL;
KeyboardKey bindSprint =                    KEY_LEFT_SHIFT;
KeyboardKey bindStrafeLeft =                KEY_A;
KeyboardKey bindStrafeRight =               KEY_D;
KeyboardKey bindWalkBackwards =             KEY_S;
KeyboardKey bindWalkForwards =              KEY_W;

// ---- gameplay ---------------------------------------------------------------
MouseButton bindAttackOrDestroy =           MOUSE_BUTTON_LEFT;
MouseButton bindPickBlock =                 MOUSE_BUTTON_MIDDLE;
MouseButton BindUseItemOrPlaceBlock =       MOUSE_BUTTON_RIGHT;

// ---- inventory --------------------------------------------------------------
KeyboardKey bindDropSelectedItem =          KEY_Q;
KeyboardKey bindHotbarSlot1 =               KEY_ONE;
KeyboardKey bindHotbarSlot2 =               KEY_TWO;
KeyboardKey bindHotbarSlot3 =               KEY_THREE;
KeyboardKey bindHotbarSlot4 =               KEY_FOUR;
KeyboardKey bindHotbarSlot5 =               KEY_FIVE;
KeyboardKey bindHotbarSlot6 =               KEY_SIX;
KeyboardKey bindHotbarSlot7 =               KEY_SEVEN;
KeyboardKey bindHotbarSlot8 =               KEY_EIGHT;
KeyboardKey bindHotbarSlot9 =               KEY_NINE;
KeyboardKey bindHotbarSlot0 =               KEY_ZERO;
KeyboardKey bindHotbarSlotKP1 =             KEY_KP_1;
KeyboardKey bindHotbarSlotKP2 =             KEY_KP_2;
KeyboardKey bindHotbarSlotKP3 =             KEY_KP_3;
KeyboardKey bindHotbarSlotKP4 =             KEY_KP_4;
KeyboardKey bindHotbarSlotKP5 =             KEY_KP_5;
KeyboardKey bindHotbarSlotKP6 =             KEY_KP_6;
KeyboardKey bindHotbarSlotKP7 =             KEY_KP_7;
KeyboardKey bindHotbarSlotKP8 =             KEY_KP_8;
KeyboardKey bindHotbarSlotKP9 =             KEY_KP_9;
KeyboardKey bindHotbarSlotKP0 =             KEY_KP_0;
KeyboardKey bindOpenOrCloseInventory =      KEY_E;
KeyboardKey bindSwapItemWithOffhand =       KEY_F;

// ---- miscellaneous ----------------------------------------------------------
KeyboardKey bindTakeScreenshot =            KEY_F2;
KeyboardKey bindToggleHUD =                 KEY_F1;
KeyboardKey bindToggleDebug =               KEY_F3;
KeyboardKey bindToggleCinematicCamera =     KEY_F6;
KeyboardKey bindToggleFullscreen =          KEY_F11;
KeyboardKey bindTogglePerspective =         KEY_F5;
KeyboardKey bindPause =                     KEY_ESCAPE;
KeyboardKey bindChatOrCommand =             KEY_SLASH;

// ---- debug & menu -----------------------------------------------------------
//TODO: navigate menus with arrow keys
KeyboardKey bindLeft =                      KEY_LEFT;
KeyboardKey bindRight =                     KEY_RIGHT;
KeyboardKey bindDown =                      KEY_DOWN;
KeyboardKey bindUp =                        KEY_UP;

