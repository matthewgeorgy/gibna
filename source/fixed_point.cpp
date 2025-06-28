#include <fixed_point.h>

s32_fp		
ToFixedPoint(f32 Value)
{
	s32		FixedPoint;


	FixedPoint = (s32)Round(Value * FP_MULTIPLIER);

	return (FixedPoint);
}

v2_fp::v2_fp()
{
}

v2_fp::v2_fp(f32 x,
			 f32 y)
{
	this->x = ToFixedPoint(x);
	this->y = ToFixedPoint(y);
}

v2_fp::v2_fp(v2 V)
{
	this->x = ToFixedPoint(V.x);
	this->y = ToFixedPoint(V.y);
}

v2_fp
operator+(v2_fp U,
		  v2_fp V)
{
	v2_fp		Result;


	Result.x = U.x + V.x;
	Result.y = U.y + V.y;

	return (Result);
}

v2_fp
operator-(v2_fp U,
		  v2_fp V)
{
	v2_fp		Result;


	Result.x = U.x - V.x;
	Result.y = U.y - V.y;

	return (Result);
}

