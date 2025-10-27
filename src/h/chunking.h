#ifndef GAME_CHUNKING_H
#define GAME_CHUNKING_H

#include <engine/h/defines.h>

#include "main.h"

#define SET_BLOCK_ID(block, id) (block = (block & ~MASK_BLOCK_ID) | id)

extern u64 CHUNKS_MAX[SET_RENDER_DISTANCE_MAX + 1];

/* chunk pointer look-up table that points to chunk_buf addresses.
 * mapping of chunk_buf entries to chunk positions in 3d space */
extern Chunk **chunk_tab;

/* chunk pointer pointer look-up table that points to chunk_tab addresses.
 * order of chunk_tab based on distance away from player */
extern Chunk ***CHUNK_ORDER;

/* queue of chunks to be processed */
extern ChunkQueue CHUNK_QUEUE;

/* player relative chunk tab access,
 * declared by the user*/
extern u32 chunk_tab_index;

/* return non-zero on failure and game_err is set accordingly */
u32 chunking_init(void);

void chunking_update(v3i16 player_delta_chunk);
void chunking_free(void);

/* index = (chunk_tab index) */
void block_place(u32 index, u32 x, u32 y, u32 z);

/* index = (chunk_tab index) */
void block_break(u32 index, u32 x, u32 y, u32 z);

/* queue_offset = first CHUNK_ORDER index to start parsing,
 * queue_size = number of indices to process in queue,
 * rate_chunk = number of chunks to process per frame,
 * rate_block = number of blocks to process per chunk per frame */
void chunk_queue_update(u32 *cursor, u32 *count, Chunk ***queue, u32 queue_id,
        u64 queue_offset, u64 queue_size, u32 rate_chunk, u32 rate_block);

void chunk_tab_shift(v3i16 player_chunk, v3i16 *player_delta_chunk);
u32 get_target_chunk_index(v3i16 player_chunk, v3i64 player_delta_target);

#ifdef FUCK // TODO: undef FUCK
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, v4u8 color);
void draw_block_wires(v3i32 pos);
void draw_bounding_box(Vector3 origin, Vector3 scl, Color col);
void draw_bounding_box_clamped(Vector3 origin, Vector3 scl, Color col);
#endif // TODO: undef FUCK

#endif /* GAME_CHUNKING_H */
