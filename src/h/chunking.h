#ifndef GAME_CHUNKING_H
#define GAME_CHUNKING_H

#include <engine/h/types.h>

#include "main.h"
#include "assets.h"

#define SET_BLOCK_ID(block, id) (block = (block & ~MASK_BLOCK_ID) | id)

/*! @brief look-up table to reduce redundant checks of untouched regions of 'chunk_buf'.
 *
 *  the sphere of chunks around 'chunk_tab' center are the only chunks that get processed,
 *  and since 'CHUNK_ORDER' is a look-up that orders 'chunk_tab' addresses based on
 *  their distance from the center index, using 'CHUNKS_MAX[render_distance]'
 *  is useful for iteration from 'CHUNK_ORDER[0]' to 'CHUNK_ORDER[CHUNKS_MAX[render_distance]]'.
 *
 *  @remark index 0 of this array is always 0 since render distance of 0 is not
 *  possible (it's possible but goofy)
 *
 *  @remark read-only, initialized internally in 'chunking_init()'.
 */
extern u64 CHUNKS_MAX[SET_RENDER_DISTANCE_MAX + 1];

/*! @brief chunk pointer look-up table that points to chunk_buf addresses.
 *
 *  'chunk_buf' addresses ordered by their positions in 3d space relative to player position.
 */
extern Chunk **chunk_tab;

/*! @brief chunk pointer pointer look-up table that points to chunk_tab addresses.
 *
 *  'chunk_tab' addresses ordered by distance from 'chunk_tab' center in ascending order.
 *
 *  @remark read-only, initialized internally in 'chunking_init()'.
 */
extern Chunk ***CHUNK_ORDER;

/*! @brief queues of chunks to be processed.
 *
 *  @remark read-only, updated internally in 'chunking_update()'.
 */
extern ChunkQueue CHUNK_QUEUE[CHUNK_QUEUES_MAX];

/*! @brief player relative chunk tab access.
 *
 *  @remark declared by the user.
 */
extern u32 chunk_tab_index;

/*! @brief initialize chunking resources.
 *
 *  allocate resources for 'chunk_buf', 'chunk_tab', 'CHUNK_ORDER' and
 *  'CHUNK_QUEUE[]' and load necessary look-ups from disk if found and build them if not found.
 *
 *  @remark building the look-ups is cpu-guzzling (performance-taxing).
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 chunking_init(void);

/*! @update everything about chunks during gameplay.
 *
 *  1. load dirty chunks into their priority queues based on their distance from
 *     the player.
 *  2. if '(flag & FLAG_MAIN_CHUNK_BUF_DIRTY)', shift 'chunk_tab'
 *     to compensate for the player crossing a chunk boundary.
 *  3. check if player has crossed more than one axis and go back to shift
 *     along that axis if true.
 *  4. find empty slots within 'settings.render_distance' and push chunks to
 *     'chunk_buf' and return that address to the respective index in 'chunk_tab'.
 *  5. remove flag 'FLAG_MAIN_CHUNK_BUF_DIRTY' when no further processing is required.
 */
void chunking_update(v3i32 player_chunk, v3i32 *player_chunk_delta);

void chunking_free(void);

/*! @param index = index into global array 'chunk_tab'.
 */
void block_place(u32 index, i32 x, i32 y, i32 z, BlockID block_id);

/*! @param index = index into global array 'chunk_tab'.
 */
void block_break(u32 index, i32 x, i32 y, i32 z);

/*! @brief translate block world position to 'chunk_buf' index relative to '*chunk'.
 *
 *  @param x, y, z = coordinates of block relative to '*chunk'.
 *
 *  @return pointer to block in 'chunk_buf'.
 */
u32 *get_block_chunk_buf_index_relative(Chunk *ch, i32 x, i32 y, i32 z);

/*! @brief translate position to 'chunk_tab' index relative to chunk position.
 *
 *  @return index into global array 'chunk_tab'.
 *  @return 'settings.chunk_tab_center' if index out of bounds.
 */
u32 get_chunk_index(v3i32 chunk, v3f64 pos);

/*! @brief get chunk relative to position.
 *
 *  @return chunk at index if 'x', 'y' and 'z' are within chunk bounds and
 *  return the correct neighboring chunk otherwise.
 */
Chunk *get_chunk_resolved(u32 index, i32 x, i32 y, i32 z);

/*! @brief get block relative to chunk.
 *
 *  @return block address in chunk if 'x', 'y' and 'z' are within chunk bounds and
 *  return the correct block in the neighboring chunk otherwise.
 */
u32 *get_block_resolved(Chunk *ch, i32 x, i32 y, i32 z);

#endif /* GAME_CHUNKING_H */
