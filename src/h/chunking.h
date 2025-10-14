#ifndef GAME_CHUNKING_H
#define GAME_CHUNKING_H

#include <engine/h/defines.h>

#include "main.h"

/* chunk buffer, raw chunk data */
extern Chunk *chunk_buf;

/* chunk pointer look-up table that points to chunk_buf addresses.
 * mapping of table entries to chunk positions in 3d space */
extern Chunk *chunk_tab[CHUNK_BUF_VOLUME];

/* chunk pointer pointer look-up table that points to chunk_tab addresses.
 * order of chunks based on distance away from player */
extern Chunk **chunk_order[CHUNK_BUF_VOLUME];

/* queue of chunks to be processed */
extern ChunkQueue chunk_queue;

/* player relative chunk tab access */
static u16 chunk_tab_index;

u8 chunking_init(void);
void chunking_update(v3i16 player_delta_chunk);
void chunking_free(void);

/* index = (chunk_tab index); */
void block_place(u32 index, u32 x, u32 y, u32 z);

/* index = (chunk_tab index); */
void block_break(u32 index, u32 x, u32 y, u32 z);

/* rate_chunk = number of chunks to process per frame,
 * rate_block = number of blocks to process per chunk per frame */
void chunk_queue_update(u32 rate_chunk, u32 rate_block);

void chunk_tab_shift(v3i16 player_chunk, v3i16 *player_delta_chunk);
u16 get_target_chunk_index(v3i16 player_chunk, v3i64 player_delta_target);
#ifdef FUCK // TODO: undef FUCK
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, v4u8 color);
void draw_block_wires(v3i32 pos);
void draw_bounding_box(Vector3 origin, Vector3 scl, Color col);
void draw_bounding_box_clamped(Vector3 origin, Vector3 scl, Color col);
#endif // TODO: undef FUCK

#endif /* GAME_CHUNKING_H */
