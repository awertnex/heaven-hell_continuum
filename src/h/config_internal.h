#ifndef HHC_CONFIG_INTERNAL_H
#define HHC_CONFIG_INTERNAL_H

#define MODE_INTERNAL_VSYNC                         0
#define MODE_INTERNAL_SKIP_TITLE_MENU               1
#define MODE_INTERNAL_LOAD_CHUNKS                   1
#define MODE_INTERNAL_EXPORT_CHUNKS                 0
#define MODE_INTERNAL_IMPORT_CHUNKS                 0

/**
 *  @brief optimization to skip fully parsing previously generated blocks in an
 *  incompletely generated chunk.
 */
#define MODE_INTERNAL_CHUNK_GENERATE_CHEAP_CHECK    1

/**
 *  @brief optimization to skip generating a chunk in the first stage if a layer
 *  is fully air. (works for the first stage because it's the terrain-height
 *  stage)
 */
#define MODE_INTERNAL_CHUNK_GENERATE_SKIP_AIR_LAYER 1

#define MODE_INTERNAL_COLLIDE                       1
#define MODE_INTERNAL_DIE                           1

#endif /* HHC_CONFIG_INTERNAL_H */
