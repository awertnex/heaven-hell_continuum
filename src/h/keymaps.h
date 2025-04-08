#ifndef KEYMAPS_H

#include "../dependencies/raylib-5.5/src/raylib.h"

// ---- movement ---------------------------------------------------------------
extern KeyboardKey bindJump;
extern KeyboardKey bindSneak;
extern KeyboardKey bindSprint;
extern KeyboardKey bindStrafeLeft;
extern KeyboardKey bindStrafeRight;
extern KeyboardKey bindWalkBackwards;
extern KeyboardKey bindWalkForwards;

// ---- gameplay ---------------------------------------------------------------
extern MouseButton bindAttackOrDestroy;
extern MouseButton bindPickBlock;
extern MouseButton BindUseItemOrPlaceBlock;

// ---- inventory --------------------------------------------------------------
extern KeyboardKey bindDropSelectedItem;
extern KeyboardKey bindHotbarSlot1;
extern KeyboardKey bindHotbarSlot2;
extern KeyboardKey bindHotbarSlot3;
extern KeyboardKey bindHotbarSlot4;
extern KeyboardKey bindHotbarSlot5;
extern KeyboardKey bindHotbarSlot6;
extern KeyboardKey bindHotbarSlot7;
extern KeyboardKey bindHotbarSlot8;
extern KeyboardKey bindHotbarSlot9;
extern KeyboardKey bindHotbarSlot0;
extern KeyboardKey bindHotbarSlotKP1;
extern KeyboardKey bindHotbarSlotKP2;
extern KeyboardKey bindHotbarSlotKP3;
extern KeyboardKey bindHotbarSlotKP4;
extern KeyboardKey bindHotbarSlotKP5;
extern KeyboardKey bindHotbarSlotKP6;
extern KeyboardKey bindHotbarSlotKP7;
extern KeyboardKey bindHotbarSlotKP8;
extern KeyboardKey bindHotbarSlotKP9;
extern KeyboardKey bindHotbarSlotKP0;
extern KeyboardKey bindOpenOrCloseInventory;
extern KeyboardKey bindSwapItemWithOffhand;

// ---- miscellaneous ----------------------------------------------------------
extern KeyboardKey bindTakeScreenshot;
extern KeyboardKey bindToggleHUD;
extern KeyboardKey bindToggleDebug;
extern KeyboardKey bindToggleCinematicCamera;
extern KeyboardKey bindToggleFullscreen;
extern KeyboardKey bindTogglePerspective;
extern KeyboardKey bindPause;
extern KeyboardKey bindChatOrCommand;

// ---- debug & menu -----------------------------------------------------------
extern KeyboardKey BIND_LEFT;
extern KeyboardKey bindRight;
extern KeyboardKey bindDown;
extern KeyboardKey bindUp;
extern KeyboardKey bindQuit;

#define KEYMAPS_H
#endif
