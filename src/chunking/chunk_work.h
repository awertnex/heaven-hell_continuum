#ifndef HHC_CHUNK_WORK_H
#define HHC_CHUNK_WORK_H

#include "deps/fossil/common/types.h"

#define CHUNK_WORK_BUDGET_DEFAULT 100000000

typedef i64 chunk_work_budget;
typedef i64 chunk_work_cost;

typedef struct hhc_chunk_work_receipt
{
    chunk_work_cost cost[13];
} hhc_chunk_work_receipt;

/*!
 *  @brief cost of work requested by chunks in a @ref hhc_chunk_scheduler.
 */
typedef enum chunk_work_cost_table
{
    CHUNK_WORK_COST_SCAN = 5,
    CHUNK_WORK_COST_PUSH = 20,
    CHUNK_WORK_COST_POP = 20,
    CHUNK_WORK_COST_IMPORT_AIR = 25,
    CHUNK_WORK_COST_IMPORT_NON_AIR = 100,
    CHUNK_WORK_COST_EXPORT_AIR = 25,
    CHUNK_WORK_COST_EXPORT_NON_AIR = 100,
    CHUNK_WORK_COST_MESH_AIR = 50,
    CHUNK_WORK_COST_MESH_NON_AIR = 600,
    CHUNK_WORK_COST_GENERATE_AIR = 250,
    CHUNK_WORK_COST_GENERATE_NON_AIR = 800,
    CHUNK_WORK_COST_GENERATE_NOISE_INIT = 20,
    CHUNK_WORK_COST_GENERATE_NOISE_SAMPLE_2D = 40,
    CHUNK_WORK_COST_GENERATE_NOISE_SAMPLE_3D = 50,
    CHUNK_WORK_COST_CHEAP_CHECK = 3
} chunk_work_cost_table;

#endif /* HHC_CHUNK_WORK_H */
