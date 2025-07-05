#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__

#include <mg.h>

#define FP_SHIFT		4
#define FP_MULTIPLIER	(1 << FP_SHIFT)

typedef s32		s32_fp;

s32_fp		ToFixedPoint(f32 Value);

struct v2_fp
{
	s32_fp		x, y;


	v2_fp();
	v2_fp(f32 x, f32 y);
	v2_fp(v2 V);
};

v2_fp		operator+(v2_fp U, v2_fp V);
v2_fp		operator-(v2_fp U, v2_fp V);

#endif // __FIXED_POINT_H__

