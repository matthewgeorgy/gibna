#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <algorithm>

#include <mg.h>
#include <bitmap.h>
#include <fixed_point.h>
#include <simd.h>
#include <stb_image.h>

#define SCR_WIDTH 	800
#define SCR_HEIGHT 	600

struct vertex
{
	v4		Pos;
	v3		Color;
	v2		TexCoord;
};

struct vertex_attribs
{
	wide_v3		Colors;
	wide_v2		TexCoords;
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

struct texture
{
	s32		Width,
			Height;
	u8		*Data;
};

struct renderer_state;

typedef vertex (*vs_proc)(renderer_state *, u32);
typedef wide_v3 (*ps_proc)(renderer_state *, vertex_attribs);

struct renderer_state
{
	buffer		VertexBuffer,
				IndexBuffer;
	texture		Texture;
	m4			WVP;
	bitmap 		*Bitmap;
	vs_proc		VS;
	ps_proc		PS;
};

struct edge
{
	static const s32 StepSizeX = SIMD_WIDTH;
	static const s32 StepSizeY = 1;

	wide_s32 OneStepX;
	wide_s32 OneStepY;

	wide_s32 Init(const v2_fp &V0, const v2_fp &V1, const v2_fp &P);
};

//**************************************
//*** "Public" *************************
//**************************************

void				Draw(renderer_state *State, u32 VertexCount);
void				DrawIndexed(renderer_state *State, u32 IndexCount);
b32					CreateBuffer(buffer *Buffer, void *Data, u32 Size);
b32					CreateTexture(texture *Texture, const char *Filename);
void				DestroyBuffer(buffer *Buffer);
void				DestroyTexture(texture *Texture);
wide_v3				SampleTexture(texture Texture, wide_v2 TexCoords);
v2					FetchV2(f32 *Vertices, u32 VertexID);
v3					FetchV3(f32 *Vertices, u32 VertexID);
v4					FetchV4(f32 *Vertices, u32 VertexID);

//**************************************
//*** "Internal" ***********************
//**************************************

s32_fp 				Orient2D(v2_fp A, v2_fp B, v2_fp C);
v2					NdcToRaster(v2 Point, s32 Width, s32 Height);
b32  				FillRule(v2_fp Edge);
void 				RasterizeTriangle(bitmap *Bitmap, triangle Triangle);
void				PerspectiveDivide(v4 *V);
vertex_attribs		InterpolateAttributes(triangle *Triangle, weights Weights);
wide_v3				ConvertIntToFloatColors(wide_v3i IntColors);
wide_v3i			ConvertFloatToIntColors(wide_v3 FloatColors);

// NOTE(matthew): See the following reference for the clipping implementation
// https://lisyarus.github.io/blog/posts/implementing-a-tiny-cpu-rasterizer-part-5.html#section-clipping
vertex				ClipIntersectEdge(vertex V0, vertex V1, f32 Value0, f32 Value1);
vertex 				*ClipTriangle(vertex *Triangle, v4 Equation, vertex *Result);
vertex 				*ClipTriangle(vertex *Begin, vertex *End);

// NOTE(matthew): scalar versions
void				RenderPixel(bitmap *Bitmap, s32 X, s32 Y, v3 PSOut);
void				UpdateDepth(u32 *BaseDepthPtr, s32 NewDepth);

// NOTE(matthew): SIMD versions
void				RenderPixels(bitmap *Bitmap, s32 X, s32 Y, wide_s32 ActivePixelMask, wide_v3 PSOut);
void				UpdateDepths(u32 *BaseDepthPtr, wide_s32 ActivePixelMask, wide_s32 OldDepth, wide_s32 NewDepth);

#endif // __RENDERER_H__

