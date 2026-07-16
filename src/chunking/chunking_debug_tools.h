#ifndef HHC_CHUNKING_DEBUG_TOOLS_H
#define HHC_CHUNKING_DEBUG_TOOLS_H

#include "deps/fossil/assets/asset_types.h"

/*!
 *  @brief convert RGBA color to 4-Byte hex color.
 *
 *  @remark color range [0.0f, 1.0f].
 *  @remark temporary, working version, til the version in the engine is fixed.
 */
#define color_v4_to_hex(r, g, b, a) \
    (((u32)((r) * 0xff) << 0x18) | \
     ((u32)((g) * 0xff) << 0x10) | \
     ((u32)((b) * 0xff) << 0x08) | \
     ((u32)((a) * 0xff) << 0x00))

#define CHUNK_DEBUG_COLOR_LOADING           color_v4_to_hex(0.701f, 0.011f, 0.019f, 0.392f)
#define CHUNK_DEBUG_COLOR_GENERATING        color_v4_to_hex(0.972f, 0.933f, 0.901f, 1.000f)
#define CHUNK_DEBUG_COLOR_MESHING           color_v4_to_hex(1.000f, 0.729f, 0.368f, 1.000f)
#define CHUNK_DEBUG_COLOR_LIGHTING          color_v4_to_hex(0.775f, 0.349f, 0.000f, 1.000f)
#define CHUNK_DEBUG_COLOR_AO                color_v4_to_hex(0.105f, 0.643f, 0.831f, 1.000f)
#define CHUNK_DEBUG_COLOR_FINISHING         color_v4_to_hex(0.500f, 1.000f, 0.400f, 1.000f)
#define CHUNK_DEBUG_COLOR_DONE              color_v4_to_hex(0.588f, 0.713f, 0.361f, 1.000f)
#define CHUNK_DEBUG_COLOR_DONE_NON_VISIBLE  color_v4_to_hex(1.000f, 1.000f, 1.000f, 0.392f)
#define CHUNK_DEBUG_COLOR_DONE_AIR          color_v4_to_hex(1.000f, 1.000f, 1.000f, 0.392f)

#define CHUNK_DEBUG_COLOR_BIAS_INFLUENCE 0.10f
#define CHUNK_GIZMO_SCALE 150.0f
#define CHUNK_GIZMO_Z_LAYER_HEIGHT 0.03f

enum chunk_debug_color_index
{
    CHUNK_DEBUG_COLOR_INDEX_NONE,
    CHUNK_DEBUG_COLOR_INDEX_LOADING,
    CHUNK_DEBUG_COLOR_INDEX_GENERATING_BASE_TERRAIN,
    CHUNK_DEBUG_COLOR_INDEX_MESHING,
    CHUNK_DEBUG_COLOR_INDEX_LIGHTING,
    CHUNK_DEBUG_COLOR_INDEX_AO,
    CHUNK_DEBUG_COLOR_INDEX_FINISHING,
    CHUNK_DEBUG_COLOR_INDEX_DONE,
    CHUNK_DEBUG_COLOR_INDEX_DONE_NON_VISIBLE,
    CHUNK_DEBUG_COLOR_INDEX_DONE_AIR,
    CHUNK_DEBUG_COLOR_COUNT
}; /* chunk_debug_color_index */

extern v4u8 chunk_debug_color[CHUNK_DEBUG_COLOR_COUNT];
extern f32 chunk_gizmo_layer_position[CHUNK_DEBUG_COLOR_COUNT];

void chunk_debug_chunk_gizmo_draw(const fsl_camera *camera);
void chunk_debug_scheduler_visualizer_draw(const fsl_camera *camera);

#endif /* HHC_CHUNKING_DEBUG_TOOLS_H */

