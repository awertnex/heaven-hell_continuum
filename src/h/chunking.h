#ifndef GAME_CHUNKING_H
#define GAME_CHUNKING_H

#include <engine/h/defines.h>

#include "main.h"
#include "settings.h"

#define CHUNK_DIAMETER  16
#define CHUNK_LAYER     (CHUNK_DIAMETER * CHUNK_DIAMETER)
#define CHUNK_VOLUME    (CHUNK_DIAMETER * CHUNK_DIAMETER * CHUNK_DIAMETER)

#define WORLD_SEA_LEVEL         0
#define WORLD_RADIUS            2048    /* chunk count */
#define WORLD_RADIUS_VERTICAL   64      /* chunk count */

#define WORLD_DIAMETER          ((WORLD_RADIUS * 2) + 1)
#define WORLD_DIAMETER_VERTICAL ((WORLD_RADIUS_VERTICAL * 2) + 1)
#define WORLD_MAX_CHUNKS \
    (WORLD_DIAMETER * WORLD_DIAMETER * WORLD_DIAMETER_VERTICAL)

#define CHUNK_BUF_RADIUS        7
#define CHUNK_BUF_DIAMETER      ((CHUNK_BUF_RADIUS * 2) + 1)
#define CHUNK_BUF_LAYER         (CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER)
#define CHUNK_BUF_VOLUME \
    (CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER)

#define CHUNK_TAB_CENTER \
    (CHUNK_BUF_RADIUS + \
     (CHUNK_BUF_RADIUS * CHUNK_BUF_DIAMETER) + \
     (CHUNK_BUF_RADIUS * CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER))

enum BlockFlags
{
    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000000 00000001] 0 */
    POSITIVE_X =    0x0000000000010000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000000 00000010] 0 */
    NEGATIVE_X =    0x0000000000020000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000000 00000100] 0 */
    POSITIVE_Y =    0x0000000000040000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000000 00001000] 0 */
    NEGATIVE_Y =    0x0000000000080000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000000 00010000] 0 */
    POSITIVE_Z =    0x0000000000100000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000000 00100000] 0 */
    NEGATIVE_Z =    0x0000000000200000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000000 01000000] 0 */
    NOT_EMPTY =     0x0000000000400000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000000 00111111] 0 */
    BLOCKFACES =    0x00000000007f0000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000011 11111111] 0 */
    BLOCKID =       0x00000000000003ff,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 01111100 00000000] 0 */
    BLOCKSTATE =    0x0000000000007c00,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 01111111 11111111] 0 */
    BLOCKDATA =     0x0000000000007fff,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00011111 00000000 00000000 00000000] 0 */
    BLOCKLIGHT =    0x000000001f000000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 10000000 00000000] 0
     * run-length encoding */
    RLE_TRIGGER =   0x0000000000008000,

    /* 63 [00000000 00000000 00000000 00001111] 32
     * 31 [00000000 00000000 00000000 00000000] 0 */
    BLOCK_X =       0x0000000f00000000,

    /* 63 [00000000 00000000 00000000 11110000] 32
     * 31 [00000000 00000000 00000000 00000000] 0 */
    BLOCK_Y =       0x000000f000000000,

    /* 63 [00000000 00000000 00001111 00000000] 32
     * 31 [00000000 00000000 00000000 00000000] 0 */
    BLOCK_Z =       0x00000f0000000000,
}; /* BlockFlags */

enum ChunkFlags
{
    FLAG_CHUNK_LOADED       = 0x01,
    FLAG_CHUNK_GENERATED    = 0x02,
    FLAG_CHUNK_RENDER       = 0x04,
    FLAG_CHUNK_DIRTY        = 0x08,

    /* chunk marking for chunk_tab shifting logic */
    FLAG_CHUNK_EDGE =   0x10,
}; /* ChunkFlags */

enum ChunkStates
{
    SHIFT_PX = 1,
    SHIFT_NX = 2,
    SHIFT_PY = 3,
    SHIFT_NY = 4,
    SHIFT_PZ = 5,
    SHIFT_NZ = 6,

    COLOR_CHUNK_LOADED  = 0x4c2607ff,
    COLOR_CHUNK_RENDER  = 0x5e7a0aff,
}; /* ChunkStates */

typedef struct Chunk
{
    v3i16 pos;  /* (world XYZ) / CHUNK_DIAMETER */
    u32 color;  /* debug color: 0xrrggbbaa */
    u64 id;     /* hash: (pos.x << 32) + (pos.y << 16) + pos.z */
    GLuint vao;
    GLuint vbo;
    u64 block[CHUNK_DIAMETER][CHUNK_DIAMETER][CHUNK_DIAMETER];
    u8 flag;
} Chunk;


/* chunk buffer, raw chunk data */
extern Chunk *chunk_buf;

/* chunk pointer look-up table */
extern Chunk *chunk_tab[CHUNK_BUF_VOLUME];

/* player relative chunk tab access */
static u16 chunk_tab_index;

static inline v3u32
index_to_coordinates_v3u32(u32 i, u64 size)
{
    return (v3u32){
        (i) % (size),
            ((i) / (size)) % (size),
            (i) / ((size) * (size)),
    };
}

static inline v3f32
index_to_coordinates_v3f32(u32 i, u64 size)
{
    return (v3f32){
        (i) % (size),
            ((i) / (size)) % (size),
            (i) / ((size) * (size)),
    };
}

static inline u32
coordinates_to_index_v3u32(v3u32 coordinates, u64 size)
{
    return (u32)
        ((coordinates.x) +
         ((coordinates.y) * (size)) +
         ((coordinates.z) * (size) * (size)));
}

static inline u8
get_mirror_axis(u8 axis)
{
    return (CHUNK_DIAMETER - 1 - (axis));
}

static inline u16
get_block_id(u32 i)
{
    return ((i) & BLOCKID);
}

static inline void
set_block_id(u32 i, u16 id)
{
    (i) = ((i) & ~BLOCKID) | (id);
}

static inline u16
get_block_state(u32 i)
{
    return (((i) & BLOCKSTATE) >> 10);
}

static inline void
set_block_state(u32 i, u16 state)
{
    ((i) = ((i) & ~BLOCKSTATE) | ((state) << 10));
}

static inline u32
get_block_data(u32 i)
{
    return ((i) & BLOCKDATA);
}

u8 init_chunking(void);
void update_chunking(v3i16 player_delta_chunk);
void free_chunking();

/* index = (chunk_tab index); */
void add_block(u32 index, u32 x, u32 y, u32 z);

/* index = (chunk_tab index); */
void remove_block(u32 index, u32 x, u32 y, u32 z);

void shift_chunk_tab(v3i16 player_chunk, v3i16 *player_delta_chunk);
u16 get_target_chunk_index(v3i16 player_chunk, v3i64 player_delta_target);
#ifdef FUCK // TODO: undef FUCK
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, v4u8 color);
void draw_block_wires(v3i32 pos);
void draw_bounding_box(Vector3 origin, Vector3 scl, Color col);
void draw_bounding_box_clamped(Vector3 origin, Vector3 scl, Color col);
#endif // TODO: undef FUCK

#endif /* GAME_CHUNKING_H */
