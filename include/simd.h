#ifndef __SIMD_H__
#define __SIMD_H__

#include <simd_4x.h>
#include <simd_8x.h>

struct weights_4x
{
	f32_4x		W0,
				W1,
				W2;
};

struct weights_8x
{
	f32_8x		W0,
				W1,
				W2;
};

#if (SIMD_WIDTH==4)

typedef s32_4x		wide_s32;
typedef f32_4x		wide_f32;
typedef v3_4x		wide_v3;
typedef weights_4x	weights;

#define WIDE_S32_ZERO_TO_RANGE	wide_s32(0, 1, 2, 3)

#elif (SIMD_WIDTH==8)

typedef s32_8x		wide_s32;
typedef f32_8x		wide_f32;
typedef v3_8x		wide_v3;
typedef weights_8x	weights;

#define WIDE_S32_ZERO_TO_RANGE	wide_s32(0, 1, 2, 3, 4, 5, 6, 7)

#else

#error Must choose a SIMD Width of 4 or 8!

#endif

#endif // __SIMD_H__

