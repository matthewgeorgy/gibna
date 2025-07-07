#ifndef __SIMD_8X_H__
#define __SIMD_8X_H__

#include <mg.h>

#include <immintrin.h>

///////////////////////////////////
// s32_8x
///////////////////////////////////

struct s32_8x
{
	__m256i		V;


	s32_8x();
	s32_8x(s32 EAll);
	s32_8x(s32 E0, s32 E1, s32 E2, s32 E3, s32 E4, s32 E5, s32 E6, s32 E7);

	s32_8x  &operator+=(s32_8x B);
	s32_8x  &operator-=(s32_8x B);
	s32_8x  &operator*=(s32_8x B);
};

// Arithmetic
s32_8x		operator+(s32_8x A, s32_8x B);
s32_8x		operator-(s32_8x A, s32_8x B);
s32_8x		operator*(s32_8x A, s32_8x B);
s32_8x		operator*(s32 A, s32_8x B);

// Comparison
s32_8x		operator==(s32_8x A, s32_8x B);
s32_8x		operator<(s32_8x A, s32_8x B);
s32_8x		operator>(s32_8x A, s32_8x B);
s32_8x		operator<=(s32_8x A, s32_8x B);
s32_8x		operator>=(s32_8x A, s32_8x B);

// Bitwise
s32_8x		operator&(s32_8x A, s32_8x B);
s32_8x		operator|(s32_8x A, s32_8x B);
s32_8x		operator<<(s32_8x A, s32 Shift);
s32_8x		operator>>(s32_8x A, s32 Shift);

// Branch mask
b32			AnyTrue(s32_8x Comparison);
b32			AllTrue(s32_8x Comparison);
b32			AllFalse(s32_8x Comparison);

///////////////////////////////////
// f32_8x
///////////////////////////////////

struct f32_8x
{
	__m256		V;


	f32_8x();
	f32_8x(f32 EAll);
	f32_8x(f32 E0, f32 E1, f32 E2, f32 E3, f32 E4, f32 E5, f32 E6, f32 E7);

	f32_8x  &operator+=(f32_8x B);
	f32_8x  &operator-=(f32_8x B);
	f32_8x  &operator*=(f32_8x B);
	f32_8x  &operator/=(f32_8x B);
};

// Arithmetic
f32_8x		operator+(f32_8x A, f32_8x B);
f32_8x		operator-(f32_8x A, f32_8x B);
f32_8x		operator*(f32_8x A, f32_8x B);
f32_8x		operator*(f32 A, f32_8x B);
f32_8x		operator/(f32_8x A, f32_8x B);

// Bitwise
f32_8x		operator&(s32_8x Mask, f32_8x B);
f32_8x		operator|(s32_8x Mask, f32_8x B);

///////////////////////////////////
// v2_8x
///////////////////////////////////

union v2_8x
{
	struct { f32_8x x, y; };
	struct { f32_8x r, g; };


	v2_8x();
	v2_8x(f32_8x x, f32_8x y);
};

v2_8x		operator+(v2_8x A, v2_8x B);
v2_8x		operator-(v2_8x A, v2_8x B);

///////////////////////////////////
// v3_8x
///////////////////////////////////

union v3_8x
{
	struct { f32_8x x, y, z; };
	struct { f32_8x r, g, b; };


	v3_8x();
	v3_8x(f32_8x x, f32_8x y, f32_8x z);
};

v3_8x		operator+(v3_8x A, v3_8x B);
v3_8x		operator-(v3_8x A, v3_8x B);

///////////////////////////////////
// v3i_8x
///////////////////////////////////

union v3i_8x
{
	struct { s32_8x x, y, z; };
	struct { s32_8x r, g, b; };


	v3i_8x();
	v3i_8x(s32_8x x, s32_8x y, s32_8x z);
};

v3i_8x		operator+(v3i_8x A, v3i_8x B);
v3i_8x		operator-(v3i_8x A, v3i_8x B);

///////////////////////////////////
// Misc
///////////////////////////////////

// NOTE(matthew):
// This keeps the value in Dest if Mask is true, and replaces it with Source
// if the Mask is false.
// Example:
//
// Mask   : 1   0   1   0   1   0   1   0
// Source : 2   5   8   6   2   5   8   6
// Dest   : 7   3   9   4   7   3   9   4
//                                       
// Out    : 7   5   9   6   7   5   9   6 
void		ConditionalAssign(s32_8x *Dest, s32_8x Mask, s32_8x Source);
s32_8x 		GatherS32(void *BasePtr, u32 Stride, s32_8x Indices);
s32_8x 		GatherU8(void *BasePtr, u32 Stride, s32_8x Indices);

// Conversion
s32_8x		WideS32FromF32(f32_8x A);
f32_8x		WideF32FromS32(s32_8x A);

#endif // __SIMD_8X_H__

