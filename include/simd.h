#ifndef __SIMD_H__
#define __SIMD_H__

#include <simd_4x.h>

typedef s32_4x		wide_s32;
typedef f32_4x		wide_f32;
typedef v3_4x		wide_v3;

#define WIDE_S32_ZERO_TO_RANGE	wide_s32(0, 1, 2, 3)

struct weights
{
	wide_f32		W0,
					W1,
					W2;
};

#endif // __SIMD_H__

