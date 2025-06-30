#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <mg.h>
#include <bitmap.h>
#include <fixed_point.h>
#include <simd.h>

#define SCR_WIDTH 	800
#define SCR_HEIGHT 	600

struct triangle
{
	v4		V0,
			V1,
			V2;
	v3		Color0,
			Color1,
			Color2;
};

struct camera
{
	v3		Pos,
			Front,
			Up;
};

struct buffer
{
	u32 		Size;
	void		*Data;
};

struct renderer_state
{
	buffer		VertexBuffer,
				IndexBuffer;
	m4			WVP;
	bitmap 		*Bitmap;
};

struct edge
{
	static const s32 StepSizeX = 4;
	static const s32 StepSizeY = 1;

	wide_s32 OneStepX;
	wide_s32 OneStepY;

	wide_s32 Init(const v2_fp &V0, const v2_fp &V1, const v2_fp &P);
};

struct color_triple
{
	v3		C0,
			C1,
			C2;
};

s32_fp 				Orient2D(v2_fp A, v2_fp B, v2_fp C);
v2					NdcToRaster(v2 Point);
b32  				FillRule(v2_fp Edge);
void 				RasterizeTriangle(bitmap *Bitmap, triangle Triangle);
void				SetPixels_4x(bitmap *Bitmap, s32 X, s32 Y, wide_s32 ActivePixelMask, weights Weights, color_triple Colors);
v4					PerspectiveDivide(v4 V);
void				Draw(renderer_state *State, u32 VertexCount);
void				DrawIndexed(renderer_state *State, u32 IndexCount);
buffer 				CreateBuffer(void *Data, u32 Size);

#endif // __RENDERER_H__

