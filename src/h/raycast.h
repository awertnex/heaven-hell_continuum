#ifndef HHC_RAYCAST_H
#define HHC_RAYCAST_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/math/vector.h"

typedef struct block_hit
{
    v3i64 pos;
    v3f64 normal;
    u16 *block;
    b8 hit;
} block_hit;

#endif /* HHC_RAYCAST_H */
