#include <simd_4x.h>

///////////////////////////////////
// s32_4x
///////////////////////////////////

s32_4x::s32_4x()
{
}

s32_4x::s32_4x(s32 EAll)
{
	this->V = _mm_set1_epi32(EAll);
}

s32_4x::s32_4x(s32 E0,
			   s32 E1,
			   s32 E2,
			   s32 E3)
{
	this->V = _mm_setr_epi32(E0, E1, E2, E3);
}

s32_4x &
s32_4x::operator+=(s32_4x B)
{
	this->V = _mm_add_epi32(this->V, B.V);

	return (*this);
}

s32_4x &
s32_4x::operator-=(s32_4x B)
{
	this->V = _mm_sub_epi32(this->V, B.V);

	return (*this);
}

s32_4x &
s32_4x::operator*=(s32_4x B)
{
	this->V = _mm_mullo_epi32(this->V, B.V);

	return (*this);
}

// Arithmetic
s32_4x
operator+(s32_4x A,
		  s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_add_epi32(A.V, B.V);

	return (Result);
}

s32_4x
operator-(s32_4x A,
		  s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_sub_epi32(A.V, B.V);

	return (Result);
}

s32_4x
operator*(s32_4x A,
		  s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_mullo_epi32(A.V, B.V);

	return (Result);
}

s32_4x		
operator*(s32 A, 
		  s32_4x B)
{
	s32_4x		Result;

	Result = s32_4x(A) * B;

	return (Result);
}

// Comparison
s32_4x		
operator==(s32_4x A,
		   s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_cmpeq_epi32(A.V, B.V);

	return (Result);
}

s32_4x		
operator<(s32_4x A, 
		  s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_cmplt_epi32(A.V, B.V);

	return (Result);
}

s32_4x		
operator>(s32_4x A, 
		  s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_cmpgt_epi32(A.V, B.V);

	return (Result);
}

s32_4x		
operator<=(s32_4x A, 
		   s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_or_si128(_mm_cmplt_epi32(A.V, B.V),
			                _mm_cmpeq_epi32(A.V, B.V));

	return (Result);
}

s32_4x	
operator>=(s32_4x A,
		   s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_or_si128(_mm_cmpgt_epi32(A.V, B.V),
			                _mm_cmpeq_epi32(A.V, B.V));

	return (Result);
}

// Bitwise
s32_4x		
operator&(s32_4x A,
		  s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_and_si128(A.V, B.V);

	return (Result);
}

s32_4x		
operator|(s32_4x A, 
		  s32_4x B)
{
	s32_4x		Result;

	Result.V = _mm_or_si128(A.V, B.V);

	return (Result);
}

s32_4x		
operator<<(s32_4x A, 
		   s32 Shift)
{
	s32_4x		Result;

	Result.V = _mm_slli_epi32(A.V, Shift);

	return (Result);
}

s32_4x		
operator>>(s32_4x A,
		   s32 Shift)
{
	s32_4x		Result;

	Result.V = _mm_srli_epi32(A.V, Shift);

	return (Result);
}

// Branch mask
b32			
AnyTrue(s32_4x Comparison)
{
	b32 Result = _mm_movemask_epi8(Comparison.V);

	return (Result);
}

b32			
AllTrue(s32_4x Comparison)
{
	b32		Result = (_mm_movemask_epi8(Comparison.V) == 15);

	return (Result);
}

b32			
AllFalse(s32_4x Comparison)
{
	b32		Result = (_mm_movemask_epi8(Comparison.V) == 0);

	return (Result);
}

///////////////////////////////////
// f32_4x
///////////////////////////////////

f32_4x::f32_4x()
{
}

f32_4x::f32_4x(f32 EAll)
{
	this->V = _mm_set1_ps(EAll);
}

f32_4x::f32_4x(f32 E0,
			   f32 E1,
			   f32 E2,
			   f32 E3)
{
	this->V = _mm_setr_ps(E0, E1, E2, E3);
}

f32_4x &
f32_4x::operator+=(f32_4x B)
{
	this->V = _mm_add_ps(this->V, B.V);

	return (*this);
}

f32_4x &
f32_4x::operator-=(f32_4x B)
{
	this->V = _mm_sub_ps(this->V, B.V);

	return (*this);
}

f32_4x &
f32_4x::operator*=(f32_4x B)
{
	this->V = _mm_mul_ps(this->V, B.V);

	return (*this);
}

f32_4x &
f32_4x::operator/=(f32_4x B)
{
	this->V = _mm_div_ps(this->V, B.V);

	return (*this);
}

// Arithmetic

f32_4x		
operator+(f32_4x A,
		  f32_4x B)
{
	f32_4x		Result;

	Result.V = _mm_add_ps(A.V, B.V);

	return (Result);
}


f32_4x		
operator-(f32_4x A, 
		  f32_4x B)
{
	f32_4x		Result;

	Result.V = _mm_sub_ps(A.V, B.V);

	return (Result);
}

f32_4x		
operator*(f32_4x A, 
		  f32_4x B)
{
	f32_4x		Result;

	Result.V = _mm_mul_ps(A.V, B.V);

	return (Result);
}

f32_4x		
operator*(f32 A, 
		  f32_4x B)
{
	f32_4x		Result;

	Result = f32_4x(A) * B;

	return (Result);
}

f32_4x		
operator/(f32_4x A,
		  f32_4x B)
{
	f32_4x		Result;

	Result.V = _mm_div_ps(A.V, B.V);

	return (Result);
}

// Bitwise
f32_4x		
operator&(s32_4x Mask, 
		  f32_4x B)
{
	f32_4x		Result;

	Result.V = _mm_and_ps(_mm_castsi128_ps(Mask.V), B.V);

	return (Result);
}

f32_4x		
operator|(s32_4x Mask, 
		  f32_4x B)
{
	f32_4x		Result;

	Result.V = _mm_or_ps(_mm_castsi128_ps(Mask.V), B.V);

	return (Result);
}

///////////////////////////////////
// v2_4x
///////////////////////////////////

v2_4x::v2_4x()
{
}

v2_4x::v2_4x(f32_4x x, 
			 f32_4x y)
{
	this->x = x;
	this->y = y;
}

v2_4x		
operator+(v2_4x A, 
		  v2_4x B)
{
	v2_4x		Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;

	return (Result);
}

v2_4x		
operator-(v2_4x A, 
		  v2_4x B)
{
	v2_4x		Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;

	return (Result);
}

///////////////////////////////////
// v3_4x
///////////////////////////////////

v3_4x::v3_4x()
{
}

v3_4x::v3_4x(f32_4x x, 
			 f32_4x y,
			 f32_4x z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

v3_4x		
operator+(v3_4x A, 
		  v3_4x B)
{
	v3_4x		Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

	return (Result);
}

v3_4x		
operator-(v3_4x A, 
		  v3_4x B)
{
	v3_4x		Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return (Result);
}

///////////////////////////////////
// v3i_4x
///////////////////////////////////

v3i_4x::v3i_4x()
{
}

v3i_4x::v3i_4x(s32_4x x, 
			   s32_4x y,
			   s32_4x z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

v3i_4x		
operator+(v3i_4x A, 
		  v3i_4x B)
{
	v3i_4x		Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

	return (Result);
}

v3i_4x		
operator-(v3i_4x A, 
		  v3i_4x B)
{
	v3i_4x		Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return (Result);
}

///////////////////////////////////
// Misc
///////////////////////////////////

void		
ConditionalAssign(s32_4x *Dest,
				  s32_4x Mask, 
				  s32_4x Source)
{
	Dest->V = _mm_or_si128(_mm_and_si128(Mask.V, Dest->V),
						  _mm_andnot_si128(Mask.V, Source.V));
}

s32_4x
GatherS32(void *BasePtr,
		  u32 Stride, 
		  s32_4x Indices)
{
	s32_4x		Result;
	u32			*V = (u32 *)&Indices.V;

	Result.V = _mm_setr_epi32(*(s32 *)((u8 *)BasePtr + V[0] * Stride),
							  *(s32 *)((u8 *)BasePtr + V[1] * Stride),
							  *(s32 *)((u8 *)BasePtr + V[2] * Stride),
							  *(s32 *)((u8 *)BasePtr + V[3] * Stride));

	return (Result);
}

s32_4x
GatherU8(void *BasePtr,
		 u32 Stride, 
		 s32_4x Indices)
{
	s32_4x		Result;
	u32			*V = (u32 *)&Indices.V;

	Result.V = _mm_setr_epi32(s32(*((u8 *)BasePtr + V[0] * Stride)),
							  s32(*((u8 *)BasePtr + V[1] * Stride)),
							  s32(*((u8 *)BasePtr + V[2] * Stride)),
							  s32(*((u8 *)BasePtr + V[3] * Stride)));

	return (Result);
}

// Conversion
s32_4x		
WideS32FromF32(f32_4x A)
{
	s32_4x		Result;

	Result.V = _mm_cvttps_epi32(A.V);

	return (Result);
}

f32_4x		
WideF32FromS32(s32_4x A)
{
	f32_4x		Result;

	Result.V = _mm_cvtepi32_ps(A.V);

	return (Result);
}

