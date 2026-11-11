#ifndef HHC_CONFIG_INTERNAL_H
#define HHC_CONFIG_INTERNAL_H

/* ---- section: video & graphics ------------------------------------------- */

#define MODE_INTERNAL_VSYNC                         0
#define MODE_INTERNAL_SKIP_TITLE_MENU               1

/* ---- section: cheats ----------------------------------------------------- */

#define MODE_INTERNAL_COLLIDE                       1
#define MODE_INTERNAL_DIE                           1

/* ---- section: chunking --------------------------------------------------- */

#define MODE_INTERNAL_LOAD_CHUNKS                   1
#define MODE_INTERNAL_EXPORT_CHUNKS                 0
#define MODE_INTERNAL_IMPORT_CHUNKS                 0

/**
 *  @brief optimization to skip generating previously generated blocks in an
 *  incompletely generated chunk.
 */
#define MODE_INTERNAL_CHUNK_GENERATE_CHEAP_CHECK    1

/**
 *  @brief optimization to skip generating a chunk in the first stage of terrain
 *  generation.
 *
 *  if a Z-layer is fully air in the first stage, terrain generation will skip
 *  to the next stage.
 */
#define MODE_INTERNAL_CHUNK_GENERATE_SKIP_AIR_LAYER 1

#endif /* HHC_CONFIG_INTERNAL_H */
