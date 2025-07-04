#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <algorithm>

#include <mg.h>
#include <bitmap.h>
#include <fixed_point.h>
#include <simd.h>

#define SCR_WIDTH 	800
#define SCR_HEIGHT 	600

struct vertex
{
	v4		Pos;
	v3		Color;
};

struct triangle
{
	vertex		V0,
				V1,
				V2;
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
	static const s32 StepSizeX = SIMD_WIDTH;
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
v4					PerspectiveDivide(v4 V);
void				Draw(renderer_state *State, u32 VertexCount);
void				DrawIndexed(renderer_state *State, u32 IndexCount);
buffer 				CreateBuffer(void *Data, u32 Size);

// NOTE(matthew): See the following reference for the clipping implementation
// https://lisyarus.github.io/blog/posts/implementing-a-tiny-cpu-rasterizer-part-5.html#section-clipping
vertex				ClipIntersectEdge(vertex V0, vertex V1, f32 Value0, f32 Value1);
vertex 				*ClipTriangle(vertex *Triangle, v4 Equation, vertex *Result);
vertex 				*ClipTriangle(vertex *Begin, vertex *End);

// NOTE(matthew): SIMD specific code
void				SetPixels(bitmap *Bitmap, s32 X, s32 Y, wide_s32 ActivePixelMask, weights Weights, color_triple Colors);
void				UpdateDepth(u32 *BaseDepthPtr, wide_s32 ActivePixelMask, wide_s32 OldDepth, wide_s32 NewDepth);

#endif // __RENDERER_H__

