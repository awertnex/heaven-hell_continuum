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

/* queues of chunks to be processed */
extern ChunkQueue CHUNK_QUEUE_1;
extern ChunkQueue CHUNK_QUEUE_2;
extern ChunkQueue CHUNK_QUEUE_3;

/* player relative chunk tab access,
 * declared by the user*/
extern u32 chunk_tab_index;

/* return non-zero on failure and game_err is set accordingly */
u32 chunking_init(void);

void chunking_update(v3i16 player_delta_chunk);
void chunking_free(void);

/* index = (chunk_tab index) */
void block_place(u32 index, u32 x, u32 y, u32 z, BlockID block_id);

/* index = (chunk_tab index) */
void block_break(u32 index, u32 x, u32 y, u32 z);

void chunk_queue_update(ChunkQueue *q);
void chunk_tab_shift(v3i16 player_chunk, v3i16 *player_delta_chunk);

/* translate block world position to chunk_buf index relative to 'chunk'.
 *
 * (x, y, z) = coordinates of block relative to 'chunk'.
 *
 * return pointer to block in 'chunk_buf' */
u32 *get_block_chunk_buf_index_relative(Chunk *chunk, i32 x, i32 y, i32 z);

u32 get_target_chunk_index(v3i16 player_chunk, v3i64 player_delta_target);
#endif /* GAME_CHUNKING_H */
