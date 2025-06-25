#ifndef SPENNYTYPE_H_
#define SPENNYTYPE_H_

#include <cstdint>

typedef uint8_t u8;
typedef int8_t i8;

typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint32_t usize;
typedef int32_t isize;
typedef uint64_t big_usize;
typedef int64_t big_isize;

typedef float f32;
typedef double f64;

// hack?
#define F32_INF (1.0f / 0.0)
#define F32_NEG_INF (-1.0f / 0.0)
typedef double f64;


#endif // SPENNYTYPE_H_
