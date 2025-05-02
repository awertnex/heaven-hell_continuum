#ifndef DEFINES_H

#include <stdint.h>
#include <stdbool.h>

#define TRUE        1
#define FALSE       0

// ---- data types -------------------------------------------------------------
typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef float		f32;
typedef double		f64;

typedef char		str;

typedef bool        b8;
typedef bool        b32;

#ifdef VECTOR2_TYPES
// ---- vector2u ---------------------------------------------------------------
typedef struct {
	u8 x, y;
} v2u8;

typedef struct {
	u16 x, y;
} v2u16;

typedef struct {
	u32 x, y;
} v2u32;

typedef struct {
	u64 x, y;
} v2u64;

// ---- vector2i ---------------------------------------------------------------
typedef struct {
	i8 x, y;
} v2i8;

typedef struct {
	i16 x, y;
} v2i16;

typedef struct {
	i32 x, y;
} v2i32;

typedef struct {
	i64 x, y;
} v2i64;

// ---- vector2f ---------------------------------------------------------------
typedef struct {
	f32 x, y;
} v2f32;

typedef struct {
	f64 x, y;
} v2f64;

#endif // VECTOR2_TYPES

#ifdef VECTOR3_TYPES
// ---- vector3u ---------------------------------------------------------------
typedef struct {
	u8 x, y, z;
} v3u8;

typedef struct {
	u16 x, y, z;
} v3u16;

typedef struct {
	u32 x, y, z;
} v3u32;

typedef struct {
	u64 x, y, z;
} v3u64;

// ---- vector3i ---------------------------------------------------------------
typedef struct {
	i8 x, y, z;
} v3i8;

typedef struct {
	i16 x, y, z;
} v3i16;

typedef struct {
	i32 x, y, z;
} v3i32;

typedef struct {
	i64 x, y, z;
} v3i64;

// ---- vector3f ---------------------------------------------------------------
typedef struct {
	f32 x, y, z;
} v3f32;

typedef struct {
	f64 x, y, z;
} v3f64;

#endif // VECTOR3_TYPES

#ifdef VECTOR4_TYPES
// ---- vector4u ---------------------------------------------------------------
typedef struct {
	u8 x, y, z, w;
} v4u8;

typedef struct {
	u16 x, y, z, w;
} v4u16;

typedef struct {
	u32 x, y, z, w;
} v4u32;

typedef struct {
	u64 x, y, z, w;
} v4u64;

// ---- vector4i ---------------------------------------------------------------
typedef struct {
	i8 x, y, z, w;
} v4i8;

typedef struct {
	i16 x, y, z, w;
} v4i16;

typedef struct {
	i32 x, y, z, w;
} v4i32;

typedef struct {
	i64 x, y, z, w;
} v4i64;

// ---- vector3f ---------------------------------------------------------------
typedef struct {
	f32 x, y, z, w;
} v4f32;

typedef struct {
	f64 x, y, z, w;
} v4f64;

#endif // VECTOR4_TYPES

#define DEFINES_H
#endif

