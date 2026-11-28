#ifndef HHC_CHUNK_WORK_H
#define HHC_CHUNK_WORK_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/common/limits.h"

#define CHUNK_WORK_BUDGET_DEFAULT 300000000
#define CHUNK_WORK_PAYCUT 1.4
#define CHUNK_RECEIPT_LINE_CAP 36
#define CHUNK_RECEIPT_CAP (CHUNK_RECEIPT_LINE_CAP * 32)

typedef i64 chunk_work_budget;
typedef i64 chunk_work_cost;

enum chunk_receipt_item
{
    CHUNK_RECEIPT_ITEM_GENERATE_TERRAIN,
    CHUNK_RECEIPT_ITEM_GENERATE_CAVES,
    CHUNK_RECEIPT_ITEM_MESH,
    CHUNK_RECEIPT_ITEM_LIGHT,
    CHUNK_RECEIPT_ITEM_AO,
    CHUNK_RECEIPT_ITEM_IMPORT,
    CHUNK_RECEIPT_ITEM_EXPORT,
    CHUNK_RECEIPT_ITEM_COUNT
}; /* chunk_receipt_item */

typedef struct hhc_chunk_receipt
{
    chunk_work_cost cost[CHUNK_RECEIPT_ITEM_COUNT];

    chunk_work_cost subtotal;
    f64 priority_rate; /* `subtotal` multiplier */
    chunk_work_cost total; /* `subtotal` multiplied by `priority_rate` */
} hhc_chunk_receipt;

typedef struct hhc_chunk_receipt_printed
{
    str cost[CHUNK_RECEIPT_ITEM_COUNT][FSL_ID_CAP];

    str subtotal[FSL_ID_CAP];
    str priority_rate[FSL_ID_CAP];
    str total[FSL_ID_CAP];

    str printed[CHUNK_RECEIPT_CAP];
} hhc_chunk_receipt_printed;

/**
 *  @brief cost of work requested by chunks in a @ref hhc_chunk_scheduler.
 */
enum chunk_work_cost_table
{
    CHUNK_WORK_COST_SCAN = 10,
    CHUNK_WORK_COST_PUSH = 15,
    CHUNK_WORK_COST_POP = 15,
    CHUNK_WORK_COST_IMPORT_AIR = 5000,
    CHUNK_WORK_COST_IMPORT_NON_AIR = 15000,
    CHUNK_WORK_COST_EXPORT_AIR = 5000,
    CHUNK_WORK_COST_EXPORT_NON_AIR = 15000,
    CHUNK_WORK_COST_CHEAP_CHECK = 15,
    CHUNK_WORK_COST_GENERATE_NOISE_INIT = 60,
    CHUNK_WORK_COST_GENERATE_NOISE_SAMPLE_2D = 120,
    CHUNK_WORK_COST_GENERATE_NOISE_SAMPLE_3D = 150,
    CHUNK_WORK_COST_MESH_AIR = 150,
    CHUNK_WORK_COST_MESH_NON_AIR = 10, /* per block */
    CHUNK_WORK_COST_LIGHT_AIR = 150,
    CHUNK_WORK_COST_LIGHT_NON_AIR = 1800,
    CHUNK_WORK_COST_AO_AIR = 150,
    CHUNK_WORK_COST_AO_NON_AIR = 2100
}; /* chunk_work_cost_table */

void chunk_work_receipt_evaluate(hhc_chunk_receipt *receipt, u32 chunk_cpi);
void chunk_work_receipt_print(const hhc_chunk_receipt *src, hhc_chunk_receipt_printed *dst);

#endif /* HHC_CHUNK_WORK_H */
