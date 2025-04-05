#ifndef CHUNKING_H

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "defines.h"

#include "main.h"
#include "setting.h"

// ---- world stuff ------------------------------------------------------------
#define WORLD_KILL_Z        -128
#define WORLD_BOTTOM        -69
#define WORLD_SEA_LEVEL     62
#define WORLD_HEIGHT_NORMAL 420 /* - WORLD_BOTTOM */
#define WORLD_HEIGHT_HELL   365 /* - WORLD_BOTTOM */
#define WORLD_HEIGHT_END    256 /* - WORLD_BOTTOM */
#define CHUNK_SIZE          64
#define WORLD_SIZE          64*32767 /* in each cardinal direction */

#define opacity 200

// ---- general ----------------------------------------------------------------
enum BlockFaces
{
    POSITIVE_X =    0x01,
    NEGATIVE_X =    0x02,
    POSITIVE_Y =    0x04,
    NEGATIVE_Y =    0x08,
    POSITIVE_Z =    0x10,
    NEGATIVE_Z =    0x20,
    NOT_EMPTY =     0x40,
    COUNTAHEAD =    0x80,
}; /* BlockFaces */

enum BlockData
{
    BLOCKFACES =    0x0000003F, // 00000000 00000000 00000000 00111111
    BLOCKSTATE =    0x00000F00, // 00000000 00000000 00001111 00000000
    BLOCKID =       0x00FFF000, // 00000000 11111111 11110000 00000000
    BLOCKLIGHT =    0x1F000000, // 00011111 00000000 00000000 00000000
}; /* BlockData */

typedef struct Chunk
{
    v2i16 pos;
    u8 i[WORLD_HEIGHT_NORMAL][CHUNK_SIZE][CHUNK_SIZE];
    u16 info[WORLD_HEIGHT_NORMAL][CHUNK_SIZE][CHUNK_SIZE];
    u8 loaded;
    /*TODO: replace 'loaded' with a 'chunk_table' array of pointers to 'chunk_buf'
      addresses and update pointer addresses as player enters or exits chunks */
} Chunk;

// ---- declarations -----------------------------------------------------------
extern u16 world_height;
extern Chunk chunk_buf[(SETTING_RENDER_DISTANCE_MAX*2) + 1][(SETTING_RENDER_DISTANCE_MAX*2) + 1];
extern Chunk *target_chunk;
extern u64 block_count; //debug mode
extern u64 quad_count; //debug mode

// ---- signatures -------------------------------------------------------------
void init_chunking();
void load_chunks();
void unload_chunks();
void add_block(Chunk *chunk, u8 x, u8 y, u16 z);
void remove_block(Chunk *chunk, u8 x, u8 y, u16 z);
void parse_chunk_states(Chunk *chunk, u16 height);
Chunk* get_chunk(v3i32 *coordinates, u16 *state, u16 flag);
void draw_chunk(Chunk *chunk, u16 height);
void draw_block(u32 block_state);
void draw_block_wires(v3i32 *pos);
void draw_bounding_box(Vector3 *origin, Vector3 *scl);
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, Color color);

#define CHUNKING_H
#endif
