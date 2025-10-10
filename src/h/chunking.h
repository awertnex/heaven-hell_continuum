#ifndef GAME_CHUNKING_H
#define GAME_CHUNKING_H

#include <engine/h/defines.h>

#include "main.h"

/* chunk buffer, raw chunk data */
extern Chunk *chunk_buf;

/* chunk pointer look-up table */
extern Chunk *chunk_tab[CHUNK_BUF_VOLUME];

/* player relative chunk tab access */
static u16 chunk_tab_index;

u8 init_chunking(void);
void update_chunking(v3i16 player_delta_chunk);
void free_chunking();

/* index = (chunk_tab index); */
void add_block(u32 index, u32 x, u32 y, u32 z);

/* index = (chunk_tab index); */
void remove_block(u32 index, u32 x, u32 y, u32 z);

/* rate = number of chunks to generate per frame */
void chunk_queue_update(u32 rate);

void shift_chunk_tab(v3i16 player_chunk, v3i16 *player_delta_chunk);
u16 get_target_chunk_index(v3i16 player_chunk, v3i64 player_delta_target);
#ifdef FUCK // TODO: undef FUCK
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, v4u8 color);
void draw_block_wires(v3i32 pos);
void draw_bounding_box(Vector3 origin, Vector3 scl, Color col);
void draw_bounding_box_clamped(Vector3 origin, Vector3 scl, Color col);
#endif // TODO: undef FUCK

#endif /* GAME_CHUNKING_H */
