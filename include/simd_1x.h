#ifndef __SIMD_1X_H__
#define __SIMD_1X_H__

#include <mg.h>

void		ConditionalAssign(s32 *Dest, s32 Mask, s32 Source);
s32 		GatherS32(void *BasePtr, u32 Stride, s32 Index);
s32			WideS32FromF32(f32 A);
f32			WideF32FromS32(s32 A);
b32			AnyTrue(s32 Comparison);

#endif // __SIMD_1X_H__
