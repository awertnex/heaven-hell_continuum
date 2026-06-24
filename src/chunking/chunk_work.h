#ifndef HHC_CHUNK_WORK_H
#define HHC_CHUNK_WORK_H

#include "deps/fossil/common/types.h"

typedef i64 chunk_work_budget;

#define CHUNK_SCHEDULERS_MAX 3

/*!
 *  @brief cost of work requested by chunks in a @ref hhc_chunk_scheduler.
 */
typedef enum chunk_work_cost
{
    CHUNK_WORK_COST_SCAN = 5,
    CHUNK_WORK_COST_PUSH = 20,
    CHUNK_WORK_COST_POP = 20,
    CHUNK_WORK_COST_IMPORT_AIR = 25,
    CHUNK_WORK_COST_IMPORT_NON_AIR = 100,
    CHUNK_WORK_COST_EXPORT = 100,
    CHUNK_WORK_COST_MESH_AIR = 2,
    CHUNK_WORK_COST_MESH_NON_AIR = 50,
    CHUNK_WORK_COST_GENERATE_AIR = 100,
    CHUNK_WORK_COST_GENERATE_NON_AIR = 500,
    CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE = 20
} chunk_work_cost;

#endif /* HHC_CHUNK_WORK_H */
