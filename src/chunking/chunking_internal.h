#ifndef HHC_CHUNKING_INTERNAL_H
#define HHC_CHUNKING_INTERNAL_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/plugins/fsl_native/torus_noise/torus_noise.h"

#include "chunk_work.h"
#include "chunking.h"

/* ---- section: definitions ------------------------------------------------ */

/**
 *  @brief count of temporary static buffers in a @ref chunk_work_func function.
 */
#define BLOCK_BUFFERS_MAX 2

/* ---- section: block flag ------------------------------------------------- */

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00000001] 00; */
#define FLAG_BLOCK_FACE_PX      0x0000000000000001

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00000010] 00; */
#define FLAG_BLOCK_FACE_NX      0x0000000000000002

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00000100] 00; */
#define FLAG_BLOCK_FACE_PY      0x0000000000000004

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00001000] 00; */
#define FLAG_BLOCK_FACE_NY      0x0000000000000008

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00010000] 00; */
#define FLAG_BLOCK_FACE_PZ      0x0000000000000010

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00100000] 00; */
#define FLAG_BLOCK_FACE_NZ      0x0000000000000020

/**
 *  @brief run-length encoding, for chunk serialization.
 *
 *  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 10000000 00000000] 00; */
#define FLAG_BLOCK_RLE          0x0000000000008000

/* ---- section: chunk ------------------------------------------------------ */

typedef void (*chunk_work_func)(hhc_chunk *chunk, chunk_work_budget budget, hhc_chunk_receipt *receipt);

/**
 *  @brief a block and all six neighbors surrounding it.
 */
typedef struct hhc_block_neighbors
{
    u16 *b, *px, *nx, *py, *ny, *pz, *nz;
} hhc_block_neighbors;

/**
 *  @brief a chunk and all six neighbors surrounding it.
 */
typedef struct hhc_chunk_neighbors
{
    hhc_chunk *ch, *px, *nx, *py, *ny, *pz, *nz;
} hhc_chunk_neighbors;

/**
 *  @brief chunk buffer, raw chunk data.
 */
typedef struct hhc_chunk_buffer
{
    /**
     *  @brief position of first empty slot in `p`.
     */
    u32 cursor;

    fsl_mem_handle handle;
    hhc_chunk *p;           /* cached pointer from `handle` */
} hhc_chunk_buffer;

/**
 *  @brief chunk-scheduler bucket for a unique distance away from @ref chunk_tab center index.
 */
typedef struct hhc_chunk_bucket
{
    u32 count;  /* number of chunks scheduled */
    u32 pos;    /* start position of bucket into @ref chunk_order.p */
    u32 len;    /* total number of slots in bucket */
    u32 push;   /* push position within bucket */
    u32 pop;    /* pop position within bucket */
} hhc_chunk_bucket;

/**
 *  @brief chunk_bucket look-up file format struct.
 */
typedef struct hhc_chunk_bucket_format
{
    u32 pos;    /* start position of bucket into @ref chunk_order.p */
    u32 len;    /* total number of slots in bucket */
} hhc_chunk_bucket_format;

/**
 *  @brief schedule of chunks to be processed.
 */
typedef struct hhc_chunk_scheduler
{
    fsl_len count;          /* number of chunks scheduled */

    fsl_mem_handle handle_p;
    fsl_mem_handle handle_bucket;
    hhc_chunk **p;          /* cached pointer from `schedule` */
    hhc_chunk_bucket *bucket; /* cached pointer from `schedule` */
    u32 buckets_max;        /* total number of members in `bucket` */
    u32 priority; /* index of current bucket being parsed */
} hhc_chunk_scheduler;

typedef struct hhc_chunk_sampler
{
    fsl_torus_sampler sampler;
    fsl_torus_sampler_context context;
} hhc_chunk_sampler;

/* ---- section: declarations ----------------------------------------------- */

extern hhc_chunk_scheduler chunk_sched;

/* ---- section: signatures ------------------------------------------------- */

/**
 *  @brief initialize resources required by chunk debug tools.
 */
u32 chunk_debug_init_internal(fsl_len chunk_count);

void chunk_debug_free_internal(void);
u32 chunk_draw_init_internal(void);
void chunk_draw_free_internal(void);

/**
 *  @brief allocate a new mesh for a chunk onto a @ref hhc_chunk_draw or update
 *  existing allocated mesh.
 */
void chunk_draw_push_internal(hhc_chunk *chunk, u32 *quad_buf, u8 *light_buf,
        u32 buf_len);

void chunk_draw_pop_internal(hhc_chunk *chunk);

/**
 *  @brief get radius of sphere squared as per internal conventions.
 */
u32 chunk_sphere_radius_get_internal(u32 radius);

hhc_chunk_neighbors chunk_neighbors_get_internal(hhc_chunk *chunk);

/**
 *  @brief chunk idle state function, do absolutely nothing.
 */
chunk_work_func chunk_work_idle_func(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt);

/**
 *  @brief load chunk from disk and generate if not found.
 */
chunk_work_func chunk_work_load_func(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt);

/**
 *  @brief generate chunk blocks.
 *
 *  automatically called from @ref chunk_load_internal().
 */
chunk_work_func chunk_work_generate_func(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt);

/**
 *  @brief make chunk mesh and upload to VRAM.
 */
chunk_work_func chunk_work_mesh_func(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt);

/**
 *  @brief calculate chunk lighting and upload to VRAM.
 */
chunk_work_func chunk_work_light_func(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt);

/**
 *  @brief calculate chunk ambient occlusion and upload to VRAM.
 */
chunk_work_func chunk_work_ao_func(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt);

/**
 *  @brief finish chunk work and return to idle state.
 */
chunk_work_func chunk_work_finish_func(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt);

/*
 *  @brief write single entry to chunk gizmo buffer.
 */
void chunk_debug_chunk_gizmo_write_internal(const hhc_chunk *chunk);

/*
 *  @brief upload entire chunk gizmo buffer to VRAM.
 */
void chunk_debug_chunk_gizmo_bake_internal(void);

#endif /* HHC_CHUNKING_INTERNAL_H */
