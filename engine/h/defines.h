#ifndef ENGINE_DEFINES_H
#define ENGINE_DEFINES_H

#include <stdint.h>

#define TRUE        1
#define FALSE       0

/* ---- data types ---------------------------------------------------------- */

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef float       f32;
typedef double      f64;

typedef char        str;

typedef uint8_t     b8;
typedef uint32_t    b32;

typedef struct buf
{
    void **i;
    void *buf;
    u64 memb;
    u64 size;
    b8 loaded;
} buf;

/* ---- vector2u ------------------------------------------------------------ */

typedef struct v2u8
{
    u8 x, y;
} v2u8;

typedef struct v2u16
{
    u16 x, y;
} v2u16;

typedef struct v2u32
{
    u32 x, y;
} v2u32;

typedef struct v2u64
{
    u64 x, y;
} v2u64;

/* ---- vector2i ------------------------------------------------------------ */

typedef struct v2i8
{
    i8 x, y;
} v2i8;

typedef struct v2i16
{
    i16 x, y;
} v2i16;

typedef struct v2i32
{
    i32 x, y;
} v2i32;

typedef struct v2i64
{
    i64 x, y;
} v2i64;

/* ---- vector2f ------------------------------------------------------------ */

typedef struct v2f32
{
    f32 x, y;
} v2f32;

typedef struct v2f64
{
    f64 x, y;
} v2f64;

/* ---- vector3u ------------------------------------------------------------ */

typedef struct v3u8
{
    u8 x, y, z;
} v3u8;

typedef struct v3u16
{
    u16 x, y, z;
} v3u16;

typedef struct v3u32
{
    u32 x, y, z;
} v3u32;

typedef struct v3u64
{
    u64 x, y, z;
} v3u64;

/* ---- vector3i ------------------------------------------------------------ */

typedef struct v3i8
{
    i8 x, y, z;
} v3i8;

typedef struct v3i16
{
    i16 x, y, z;
} v3i16;

typedef struct v3i32
{
    i32 x, y, z;
} v3i32;

typedef struct v3i64
{
    i64 x, y, z;
} v3i64;

/* ---- vector3f ------------------------------------------------------------ */

typedef struct v3f32
{
    f32 x, y, z;
} v3f32;

typedef struct v3f64
{
    f64 x, y, z;
} v3f64;

/* ---- vector4u ------------------------------------------------------------ */

typedef struct v4u8
{
    u8 x, y, z, w;
} v4u8;

typedef struct v4u16
{
    u16 x, y, z, w;
} v4u16;

typedef struct v4u32
{
    u32 x, y, z, w;
} v4u32;

typedef struct v4u64
{
    u64 x, y, z, w;
} v4u64;

/* ---- vector4i ------------------------------------------------------------ */

typedef struct v4i8
{
    i8 x, y, z, w;
} v4i8;

typedef struct v4i16
{
    i16 x, y, z, w;
} v4i16;

typedef struct v4i32
{
    i32 x, y, z, w;
} v4i32;

typedef struct v4i64
{
    i64 x, y, z, w;
} v4i64;

/* ---- vector4f ------------------------------------------------------------ */

typedef struct v4f32
{
    f32 x, y, z, w;
} v4f32;

typedef struct v4f64
{
    f64 x, y, z, w;
} v4f64;

/* ---- matrix4f ------------------------------------------------------------ */

typedef struct m4f32
{
    f32
        a11, a12, a13, a14,
        a21, a22, a23, a24,
        a31, a32, a33, a34,
        a41, a42, a43, a44;
} m4f32;

#endif /* ENGINE_DEFINES_H */
