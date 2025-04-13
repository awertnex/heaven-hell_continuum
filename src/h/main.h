#ifndef MINECRAFT_H

#define MC_C_VERSION "Minecraft.c 0.1.4"
#define MC_C_AUTHOR "Author: Lily Awertnex"

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "defines.h"

// ---- platform layer ---------------------------------------------------------
#define WIDTH 1280
#define HEIGHT 720
#define MARGIN 20

#define MODE_DEBUG          1
#define MODE_COLLIDE        0
#define MODE_GRAVITY        1
#define MODE_GRAY_BLOCKS    1

// ---- declarations -----------------------------------------------------------
extern f64 deltaTime;
extern f64 gameStartTime;
extern u64 gameTick;
extern u64 gameDays;
#define dt GetFrameTime()

extern u16 state;
extern u8 stateMenuDepth;

extern f64 skyboxMidDay;
extern f64 skyboxBurn;
extern f64 skyboxBurnBoost;
extern f64 skyboxMidNight;

// ---- signatures -------------------------------------------------------------
int mc_mkdir(const char *path, u16 mode);
void init_world();

#define MINECRAFT_H
#endif
