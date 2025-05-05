#ifndef MC_C_CHUNKING_H

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "../engine/h/defines.h"

#include "main.h"
#include "setting.h"

// ---- world stuff ------------------------------------------------------------
#define WORLD_KILL_Z        -128
#define WORLD_BOTTOM        -69
#define WORLD_SEA_LEVEL     62
#define WORLD_HEIGHT_NORMAL 420 // - WORLD_BOTTOM
#define WORLD_HEIGHT_HELL   365 // - WORLD_BOTTOM
#define WORLD_HEIGHT_END    256 // - WORLD_BOTTOM

#define CHUNK_DIAMETER      32
#define CHUNK_DATA_SIZE     (sizeof(Chunk)) // struct chunk
#define CHUNK_BUF_RADIUS    (SETTING_RENDER_DISTANCE_MAX)
#define CHUNK_BUF_DIAMETER  ((CHUNK_BUF_RADIUS * 2) + 1)
#define CHUNK_BUF_ELEMENTS  (CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER)
#define CHUNK_TAB_CENTER    (CHUNK_BUF_RADIUS + (CHUNK_BUF_RADIUS * CHUNK_BUF_DIAMETER))

#define WORLD_RADIUS        32767 // (((int16_t top of range) - 1)/2)
#define WORLD_DIAMETER      ((WORLD_RADIUS * 2) + 1)
#define WORLD_AREA          (CHUNK_DIAMETER * WORLD_DIAMETER * WORLD_DIAMETER)
#define WORLD_MAX_CHUNKS    (WORLD_AREA / CHUNK_DIAMETER)
#define CHUNK_MAX_BLOCKS    (CHUNK_DIAMETER * CHUNK_DIAMETER * WORLD_HEIGHT_NORMAL)
#define CHUNK_MAX_QUADS     ((CHUNK_DIAMETER / 2) * CHUNK_DIAMETER * WORLD_HEIGHT_NORMAL)
#define CHUNK_MAX_TRIS      (CHUNK_MAX_QUADS * 2)
#define CHUNK_MAX_VERTS     (CHUNK_MAX_QUADS * 4)

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
    // COUNTAHEAD: fwrite(): if ((chunk.i[n] & NOT_EMPTY) && chunk.info[n] == chunk.info[n - 1]) loop: u32 count++, compare again; if comparison fails, byte[n] |= COUNTAHEAD; 4 bytes[n + 1] = count;
    // COUNTAHEAD: fread(): if (byte[n] & COUNTAHEAD) u32 count = next 4 bytes, fill chunk.i from chunk.i[n] to chunk.i[n + count]
}; /* BlockFaces */

enum BlockData
{
    BLOCKFACES =    0x0000003f, // 00000000 00000000 00000000 00111111
    BLOCKID =       0x000fff00, // 00000000 00001111 11111111 00000000
    BLOCKSTATE =    0x00f00000, // 00000000 11110000 00000000 00000000
    BLOCKDATA =     0x00ffff00, // 00000000 11111111 11111111 00000000
    BLOCKLIGHT =    0x1f000000, // 00011111 00000000 00000000 00000000
}; /* BlockData */

#define GET_CHUNK_TAB_INDEX(x, y)   (x + (y * CHUNK_BUF_DIAMETER))
#define GET_CHUNK_XY(x, y)          (x + (y * CHUNK_DATA_SIZE))
#define GET_BLOCK_XYZ(x, y, z)      (x + (y * CHUNK_DIAMETER) + (z * CHUNK_DIAMETER * CHUNK_DIAMETER))
#define GET_BLOCKID(i)              (((i) & BLOCKID) >> 8)
#define SET_BLOCKID(i, value)       ((i) = ((i) & ~BLOCKID) | ((value) << 8))
#define GET_BLOCKSTATE(i)           (((i) & BLOCKSTATE) >> 20)
#define SET_BLOCKSTATE(i, value)    ((i) = ((i) & ~BLOCKSTATE) | ((value) << 20))
#define GET_BLOCKDATA(i)            (((i) & BLOCKDATA) >> 8)

enum ChunkStates
{
    STATE_CHUNK_LOADED = 1,
    STATE_CHUNK_RENDER = 2,
    STATE_CHUNK_DIRTY = 3,
}; /* ChunkStates */

// TODO: add 'version' byte to the chunk file for evolving the format safely
// TODO: add chunk slicing
typedef struct Chunk
{
    v2i16 pos;
    u32 id;
    u32 i[WORLD_HEIGHT_NORMAL][CHUNK_DIAMETER][CHUNK_DIAMETER];
    Model model;
    Material mat;
    u8 state;
} Chunk;

// ---- declarations -----------------------------------------------------------
extern u16 world_height;
extern Chunk *chunk_buf;
extern void *chunk_tab[CHUNK_BUF_ELEMENTS];
extern Chunk *target_chunk;
extern struct WorldStats
{
    u64 block_count;
    u64 quad_count;
} world_stats;
extern u8 opacity;

// ---- signatures -------------------------------------------------------------
u8 init_chunking();
void free_chunking();

void add_block(Chunk *chunk, u8 x, u8 y, u16 z);
void remove_block(Chunk *chunk, u8 x, u8 y, u16 z);
void load_chunk(Chunk *chunk);
void update_chunk();
void unload_chunk();
void update_chunk_buf(v2i16 *player_chunk);
Chunk *get_chunk(v3i32 *coordinates, u16 *state, u16 flag);
void draw_chunk_buffer(Chunk *chunk_buf);
void draw_chunk(Chunk *chunk);
void draw_block(u32 block_states);
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, Color color);
void draw_block_wires(v3i32 pos);
void draw_bounding_box(Vector3 origin, Vector3 scl);
void draw_bounding_box_clamped(Vector3 origin, Vector3 scl);

#define MC_C_CHUNKING_H
#endif

