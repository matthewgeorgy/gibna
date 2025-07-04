#include <simd_8x.h>

///////////////////////////////////
// s32_8x
///////////////////////////////////

s32_8x::s32_8x()
{
}

s32_8x::s32_8x(s32 EAll)
{
	this->V = _mm256_set1_epi32(EAll);
}

s32_8x::s32_8x(s32 E0, s32 E1, s32 E2, s32 E3, s32 E4, s32 E5, s32 E6, s32 E7)
{
	this->V = _mm256_setr_epi32(E0, E1, E2, E3, E4, E5, E6, E7);
}

s32_8x &
s32_8x::operator+=(s32_8x B)
{
	this->V = _mm256_add_epi32(this->V, B.V);

	return (*this);
}

s32_8x &
s32_8x::operator-=(s32_8x B)
{
	this->V = _mm256_sub_epi32(this->V, B.V);

	return (*this);
}

s32_8x &
s32_8x::operator*=(s32_8x B)
{
	this->V = _mm256_mullo_epi32(this->V, B.V);

	return (*this);
}

// Arithmetic
s32_8x
operator+(s32_8x A,
		  s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_add_epi32(A.V, B.V);

	return (Result);
}

s32_8x
operator-(s32_8x A,
		  s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_sub_epi32(A.V, B.V);

	return (Result);
}

s32_8x
operator*(s32_8x A,
		  s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_mullo_epi32(A.V, B.V);

	return (Result);
}

s32_8x
operator*(s32 A,
		  s32_8x B)
{
	s32_8x		Result;

	Result = s32_8x(A) * B;

	return (Result);
}

// Comparison
s32_8x
operator==(s32_8x A,
		   s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_cmpeq_epi32(A.V, B.V);

	return (Result);
}

s32_8x
operator<(s32_8x A,
		  s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_cmpgt_epi32(B.V, A.V);

	return (Result);
}

s32_8x
operator>(s32_8x A,
		  s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_cmpgt_epi32(A.V, B.V);

	return (Result);
}

s32_8x
operator<=(s32_8x A,
		   s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_or_si256(_mm256_cmpgt_epi32(B.V, A.V),
							   _mm256_cmpeq_epi32(A.V, B.V));

	return (Result);
}

s32_8x
operator>=(s32_8x A,
		   s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_or_si256(_mm256_cmpgt_epi32(A.V, B.V),
							   _mm256_cmpeq_epi32(A.V, B.V));

	return (Result);
}

// Bitwise
s32_8x
operator&(s32_8x A,
		  s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_and_si256(A.V, B.V);

	return (Result);
}

s32_8x
operator|(s32_8x A,
		  s32_8x B)
{
	s32_8x		Result;

	Result.V = _mm256_or_si256(A.V, B.V);

	return (Result);
}

s32_8x
operator<<(s32_8x A,
		   s32 Shift)
{
	s32_8x		Result;

	Result.V = _mm256_slli_epi32(A.V, Shift);

	return (Result);
}

s32_8x
operator>>(s32_8x A,
		   s32 Shift)
{
	s32_8x		Result;

	Result.V = _mm256_srli_epi32(A.V, Shift);

	return (Result);
}

// Branch mask
b32
AnyTrue(s32_8x Comparison)
{
	b32 Result = _mm256_movemask_epi8(Comparison.V);

	return (Result);
}

b32
AllTrue(s32_8x Comparison)
{
	b32 Result = (_mm256_movemask_epi8(Comparison.V) == 15);

	return (Result);
}

b32
AllFalse(s32_8x Comparison)
{
	b32 Result = (_mm256_movemask_epi8(Comparison.V) == 0);

	return (Result);
}

///////////////////////////////////
// f32_8x
///////////////////////////////////

f32_8x::f32_8x()
{
}

f32_8x::f32_8x(f32 EAll)
{
	this->V = _mm256_set1_ps(EAll);
}

f32_8x::f32_8x(f32 E0, f32 E1, f32 E2, f32 E3, f32 E4, f32 E5, f32 E6, f32 E7)
{
	this->V = _mm256_setr_ps(E0, E1, E2, E3, E4, E5, E6, E7);
}

f32_8x &
f32_8x::operator+=(f32_8x B)
{
	this->V = _mm256_add_ps(this->V, B.V);

	return (*this);
}

f32_8x &
f32_8x::operator-=(f32_8x B)
{
	this->V = _mm256_sub_ps(this->V, B.V);

	return (*this);
}

f32_8x &
f32_8x::operator*=(f32_8x B)
{
	this->V = _mm256_mul_ps(this->V, B.V);

	return (*this);
}

f32_8x &
f32_8x::operator/=(f32_8x B)
{
	this->V = _mm256_div_ps(this->V, B.V);

	return (*this);
}

// Arithmetic
f32_8x
operator+(f32_8x A,
		  f32_8x B)
{
	f32_8x		Result;

	Result.V = _mm256_add_ps(A.V, B.V);

	return (Result);
}

f32_8x
operator-(f32_8x A,
		  f32_8x B)
{
	f32_8x		Result;

	Result.V = _mm256_sub_ps(A.V, B.V);

	return (Result);
}

f32_8x
operator*(f32_8x A,
		  f32_8x B)
{
	f32_8x		Result;

	Result.V = _mm256_mul_ps(A.V, B.V);

	return (Result);
}

f32_8x
operator*(f32 A,
		  f32_8x B)
{
	f32_8x		Result;

	Result = f32_8x(A) * B;

	return (Result);
}

f32_8x
operator/(f32_8x A,
		  f32_8x B)
{
	f32_8x		Result;

	Result.V = _mm256_div_ps(A.V, B.V);

	return (Result);
}

///////////////////////////////////
// v3_8x
///////////////////////////////////

v3_8x::v3_8x()
{
}

v3_8x::v3_8x(f32_8x x,
			 f32_8x y,
			 f32_8x z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

v3_8x
operator+(v3_8x A, v3_8x B)
{
	v3_8x		Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

	return (Result);
}

v3_8x
operator-(v3_8x A, v3_8x B)
{
	v3_8x		Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return (Result);
}

///////////////////////////////////
// Misc
///////////////////////////////////

void
ConditionalAssign(s32_8x *Dest,
				  s32_8x Mask,
				  s32_8x Source)
{
	Dest->V = _mm256_or_si256(_mm256_and_si256(Mask.V, Dest->V),
							  _mm256_andnot_si256(Mask.V, Source.V));
}

s32_8x
GatherS32(void *BasePtr,
		  u32 Stride,
		  s32_8x Indices)
{
	s32_8x		Result;
	u32			*V = (u32 *)&Indices.V;

	Result.V = _mm256_setr_epi32(*(s32 *)((u8 *)BasePtr + V[0] * Stride),
							     *(s32 *)((u8 *)BasePtr + V[1] * Stride),
							     *(s32 *)((u8 *)BasePtr + V[2] * Stride),
							     *(s32 *)((u8 *)BasePtr + V[3] * Stride),
							     *(s32 *)((u8 *)BasePtr + V[4] * Stride),
							     *(s32 *)((u8 *)BasePtr + V[5] * Stride),
							     *(s32 *)((u8 *)BasePtr + V[6] * Stride),
							     *(s32 *)((u8 *)BasePtr + V[7] * Stride));

	return (Result);
}

// Conversion
s32_8x
WideS32FromF32(f32_8x A)
{
	s32_8x		Result;

	Result.V = _mm256_cvttps_epi32(A.V);

	return (Result);
}

f32_8x
WideF32FromS32(s32_8x A)
{
	f32_8x		Result;

	Result.V = _mm256_cvtepi32_ps(A.V);

	return (Result);
}

