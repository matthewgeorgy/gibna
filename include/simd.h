#ifndef __SIMD_H__
#define __SIMD_H__

#include <simd_4x.h>

typedef s32_4x		wide_s32;
typedef f32_4x		wide_f32;

#define WIDE_S32_ZERO_TO_RANGE	wide_s32(0, 1, 2, 3)

#endif // __SIMD_H__

