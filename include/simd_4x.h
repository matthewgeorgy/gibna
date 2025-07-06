#ifndef __SIMD_4X_H__
#define __SIMD_4X_H__

#include <mg.h>

#include <immintrin.h>

///////////////////////////////////
// s32_4x
///////////////////////////////////

struct s32_4x
{
	__m128i		V;


	s32_4x();
	s32_4x(s32 EAll);
	s32_4x(s32 E0, s32 E1, s32 E2, s32 E3);

	s32_4x  &operator+=(s32_4x B);
	s32_4x  &operator-=(s32_4x B);
	s32_4x  &operator*=(s32_4x B);
};

// Arithmetic
s32_4x		operator+(s32_4x A, s32_4x B);
s32_4x		operator-(s32_4x A, s32_4x B);
s32_4x		operator*(s32_4x A, s32_4x B);
s32_4x		operator*(s32 A, s32_4x B);

// Comparison
s32_4x		operator==(s32_4x A, s32_4x B);
s32_4x		operator<(s32_4x A, s32_4x B);
s32_4x		operator>(s32_4x A, s32_4x B);
s32_4x		operator<=(s32_4x A, s32_4x B);
s32_4x		operator>=(s32_4x A, s32_4x B);

// Bitwise
s32_4x		operator&(s32_4x A, s32_4x B);
s32_4x		operator|(s32_4x A, s32_4x B);
s32_4x		operator<<(s32_4x A, s32 Shift);
s32_4x		operator>>(s32_4x A, s32 Shift);

// Branch mask
b32			AnyTrue(s32_4x Comparison);
b32			AllTrue(s32_4x Comparison);
b32			AllFalse(s32_4x Comparison);

///////////////////////////////////
// f32_4x
///////////////////////////////////

struct f32_4x
{
	__m128		V;


	f32_4x();
	f32_4x(f32 EAll);
	f32_4x(f32 E0, f32 E1, f32 E2, f32 E3);

	f32_4x  &operator+=(f32_4x B);
	f32_4x  &operator-=(f32_4x B);
	f32_4x  &operator*=(f32_4x B);
	f32_4x  &operator/=(f32_4x B);
};

// Arithmetic
f32_4x		operator+(f32_4x A, f32_4x B);
f32_4x		operator-(f32_4x A, f32_4x B);
f32_4x		operator*(f32_4x A, f32_4x B);
f32_4x		operator*(f32 A, f32_4x B);
f32_4x		operator/(f32_4x A, f32_4x B);

///////////////////////////////////
// v3_4x
///////////////////////////////////

union v3_4x
{
	struct { f32_4x x, y, z; };
	struct { f32_4x r, g, b; };


	v3_4x();
	v3_4x(f32_4x x, f32_4x y, f32_4x z);
};

v3_4x		operator+(v3_4x A, v3_4x B);
v3_4x		operator-(v3_4x A, v3_4x B);

///////////////////////////////////
// Misc
///////////////////////////////////

// NOTE(matthew):
// This keeps the value in Dest if Mask is true, and replaces it with Source
// if the Mask is false.
// Example:
//
// Mask   : 1   0   1   0
// Source : 2   5   8   6
// Dest   : 7   3   9   4
//
// Out    : 7   5   9   6 
void		ConditionalAssign(s32_4x *Dest, s32_4x Mask, s32_4x Source);
s32_4x 		GatherS32(void *BasePtr, u32 Stride, s32_4x Indices);
s32_4x 		GatherU8(void *BasePtr, u32 Stride, s32_4x Indices);

// Conversion
s32_4x		WideS32FromF32(f32_4x A);
f32_4x		WideF32FromS32(s32_4x A);

#endif // __SIMD_4X_H__

