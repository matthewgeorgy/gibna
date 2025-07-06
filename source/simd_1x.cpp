#include <simd_1x.h>

void
ConditionalAssign(s32 *Dest,
				  s32 Mask,
				  s32 Source)
{
	if (Mask == 0)
	{
		*Dest = Source;
	}
}

s32
GatherS32(void *BasePtr,
		  u32 Stride,
		  s32 Index)
{
	s32 Value;

	Value = *(s32 *)((u8 *)BasePtr + Stride * Index);

	return (Value);
}

s32
GatherU8(void *BasePtr,
		 u32 Stride,
		 s32 Index)
{
	s32 Value;

	Value = s32(*((u8 *)BasePtr + Stride * Index));

	return (Value);
}

s32
WideS32FromF32(f32 A)
{
	return (s32(A));
}

f32
WideF32FromS32(s32 A)
{
	return (f32(A));
}

b32			
AnyTrue(s32 Comparison)
{
	return (Comparison);
}

