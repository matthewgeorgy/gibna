#define _CRT_SECURE_NO_WARNINGS
#include <renderer.h>

void
RasterizeTriangle(renderer_state *State,
				  triangle Triangle)
{
	s32				MinX, MaxX,
					MinY, MaxY;

	v2 RasterV0 = NdcToRaster(v2(Triangle.V0.Pos.x, Triangle.V0.Pos.y));
	v2 RasterV1 = NdcToRaster(v2(Triangle.V1.Pos.x, Triangle.V1.Pos.y));
	v2 RasterV2 = NdcToRaster(v2(Triangle.V2.Pos.x, Triangle.V2.Pos.y));

	v2_fp V0 = v2_fp(RasterV0);
	v2_fp V1 = v2_fp(RasterV1);
	v2_fp V2 = v2_fp(RasterV2);

	// Triangle bounding box
	// NOTE(matthew): Fixed-point was causing artifacting due to us (potentially)
    // skipping over the last pixel in the row. This is because if the fractional
	// part is sufficiently large, it means that we should be shading this pixel;
	// but shifting removes this information.
	// Thus, we simply add a +1 offset to "pad" the triangle by one row and
	// one column.
	MinX = ((s32)(Min(Min(V0.x, V1.x), V2.x)) >> FP_SHIFT);
	MaxX = ((s32)(Max(Max(V0.x, V1.x), V2.x)) >> FP_SHIFT) + 1;
	MinY = ((s32)(Min(Min(V0.y, V1.y), V2.y)) >> FP_SHIFT);
	MaxY = ((s32)(Max(Max(V0.y, V1.y), V2.y)) >> FP_SHIFT) + 1;
	
	// Align (round DOWN) starting pixel in X to SIMD width to prevent 
	// overwriting into the next row.
	// Eg, if MinX = 13 and SIMD_WIDTH=4, then new MinX = 12.
	MinX = (MinX - (SIMD_WIDTH - 1)) & ~(SIMD_WIDTH - 1);

	// Screen clipping
	MinX = Max(MinX, 0);
	MaxX = Min(MaxX, SCR_WIDTH);
	MinY = Max(MinY, 0);
	MaxY = Min(MaxY, SCR_HEIGHT);

	// Initial edge function values
	v2_fp Pixel = v2_fp(f32(MinX) + 0.5f, f32(MinY) + 0.5f);
	edge E01, E12, E20;
	wide_s32 WideZero = wide_s32(0);
	wide_s32 WideOne = wide_s32(1);

	wide_s32 W0Row = E12.Init(V2, V1, Pixel);
	wide_s32 W1Row = E20.Init(V0, V2, Pixel);
	wide_s32 W2Row = E01.Init(V1, V0, Pixel);

	if (FillRule(V2 - V1))	W0Row -= WideOne;
	if (FillRule(V0 - V2))	W1Row -= WideOne;
	if (FillRule(V1 - V0))	W2Row -= WideOne;

	for (s32 Y = MinY; Y < MaxY; Y += edge::StepSizeY)
	{
		wide_s32 W0 = W0Row;
		wide_s32 W1 = W1Row;
		wide_s32 W2 = W2Row;

		for (s32 X = MinX; X < MaxX; X += edge::StepSizeX)
		{
			wide_s32 ColorMask = (W0 | W1 | W2) >= WideZero;

			if (AnyTrue(ColorMask))
			{
				// Barycentric weights
				wide_f32 L0 = WideF32FromS32(W0) * Triangle.V0.Pos.w;
				wide_f32 L1 = WideF32FromS32(W1) * Triangle.V1.Pos.w;
				wide_f32 L2 = WideF32FromS32(W2) * Triangle.V2.Pos.w;
				wide_f32 InvSum = wide_f32(1) / (L0 + L1 + L2);

				weights Weights;

				Weights.W0 = L0 * InvSum;
				Weights.W1 = L1 * InvSum;
				Weights.W2 = L2 * InvSum;

				// Depth
				wide_f32 Z = Weights.W0 * Triangle.V0.Pos.z +
							 Weights.W1 * Triangle.V1.Pos.z +
							 Weights.W2 * Triangle.V2.Pos.z;
				wide_f32 MaxDepthValue = WideF32FromS32(wide_s32(0x7FFFFFFF));
				wide_s32 NewDepth = WideS32FromF32((wide_f32(0.5f) + wide_f32(0.5f) * Z) * MaxDepthValue);

				s32 DepthPixelCoord = X + Y * State->Bitmap->Width;
				u32 *BaseDepthPtr = &State->Bitmap->DepthBuffer[DepthPixelCoord];
				wide_s32 OldDepth = GatherS32(BaseDepthPtr, sizeof(u32), WIDE_S32_ZERO_TO_RANGE);

				wide_s32 DepthMask = NewDepth < OldDepth;
				wide_s32 ActivePixelMask = ColorMask & DepthMask;

				if (AnyTrue(ActivePixelMask))
				{
					// NOTE(matthew): THESE ARE NOW TEXTURE COORDINATES
					color_triple Colors;

					Colors.C0 = Triangle.V0.Color;
					Colors.C1 = Triangle.V1.Color;
					Colors.C2 = Triangle.V2.Color;

					// NOTE(matthew): When going wide, we may have to touch pixels that are
					// outside of the triangle. These will have _negative_ barycentric weights,
					// thus producing invalid (or even out-of-bounds) vertex attributes.
					// To mitigate this, we simply AND the weights with our active pixel
					// mask. This will leave valid valid weights untouched, while setting the
					// invalid ones to 0. And since we conditionally overwrite the framebuffer
					// using the active pixel mask, these "null" pixels produced by 0 weights
					// will never be seen :)	
#if (SIMD_WIDTH != 1)
					Weights.W0 = ActivePixelMask & Weights.W0;
					Weights.W1 = ActivePixelMask & Weights.W1;
					Weights.W2 = ActivePixelMask & Weights.W2;
#endif

					// TODO(matthew): do a pixel shader, have it return a wide_v3
					SetPixels(State, X, Y, ActivePixelMask, Weights, Colors);

					UpdateDepth(BaseDepthPtr, ActivePixelMask, OldDepth, NewDepth);
				}
			}

			W0 += E12.OneStepX;
			W1 += E20.OneStepX;
			W2 += E01.OneStepX;
		}

		W0Row += E12.OneStepY;
		W1Row += E20.OneStepY;
		W2Row += E01.OneStepY;
	}
}

s32_fp
Orient2D(v2_fp A,
		 v2_fp B,
		 v2_fp C)
{
	s32_fp		Det;


	Det = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);

	return (Det);
}

v2
NdcToRaster(v2 Point)
{
	v2		Pixel;


	Pixel.x = (Point.x + 1.0f) * 0.5f * SCR_WIDTH;
	Pixel.y = (Point.y + 1.0f) * 0.5f * SCR_HEIGHT;

	return (Pixel);
}

b32
FillRule(v2_fp Edge)
{
	b32 IsTopLeft = Edge.y > 0;
	b32 IsTopEdge = (Edge.y == 0) && (Edge.x < 0);

	return (IsTopLeft || IsTopEdge);
}

wide_s32
edge::Init(const v2_fp &V0,
		   const v2_fp &V1,
		   const v2_fp &P)
{
	s32_fp A = (V0.y - V1.y);
	s32_fp B = (V1.x - V0.x);
	s32_fp C = (V0.x * V1.y) - (V0.y * V1.x);

	this->OneStepX = wide_s32((A << FP_SHIFT) * StepSizeX);
	this->OneStepY = wide_s32((B << FP_SHIFT) * StepSizeY);

	wide_s32 X = wide_s32(P.x) + WIDE_S32_ZERO_TO_RANGE * wide_s32(FP_MULTIPLIER);
	wide_s32 Y = wide_s32(P.y);

	wide_s32 InitialEdgeValue = (wide_s32(A) * X + wide_s32(B) * Y + wide_s32(C));

	return (InitialEdgeValue);
}

void
SetPixels(renderer_state *State,
		  s32 X,
		  s32 Y,
		  wide_s32 ActivePixelMask,
		  weights Weights,
		  color_triple Colors)
{
	wide_v3 TexCoord0 = wide_v3(Colors.C0.r * Weights.W0, Colors.C0.g * Weights.W0, 0);
	wide_v3 TexCoord1 = wide_v3(Colors.C1.r * Weights.W1, Colors.C1.g * Weights.W1, 0);
	wide_v3 TexCoord2 = wide_v3(Colors.C2.r * Weights.W2, Colors.C2.g * Weights.W2, 0);
	wide_v3 TexCoords = TexCoord0 + TexCoord1 + TexCoord2;

	wide_s32 TextureCoordX = WideS32FromF32(TexCoords.x * WideF32FromS32(State->Texture.Width));
	wide_s32 TextureCoordY = WideS32FromF32(TexCoords.y * WideF32FromS32(State->Texture.Height));
	wide_s32 TexelIndices = TextureCoordX + TextureCoordY * wide_s32(State->Texture.Width);

	u8 *BaseTexturePtr = &State->Texture.Data[0];

	wide_s32 NewReds   = GatherU8(BaseTexturePtr + 0, BYTES_PER_PIXEL, TexelIndices);
	wide_s32 NewGreens = GatherU8(BaseTexturePtr + 1, BYTES_PER_PIXEL, TexelIndices);
	wide_s32 NewBlues  = GatherU8(BaseTexturePtr + 2, BYTES_PER_PIXEL, TexelIndices);

	wide_s32 PixelIndices = WIDE_S32_ZERO_TO_RANGE;
	s32 PixelCoord = (X + Y * State->Bitmap->Width) * BYTES_PER_PIXEL;
	u8 *BasePixelPtr = &State->Bitmap->ColorBuffer[PixelCoord];

	wide_s32 OldReds   = GatherS32(BasePixelPtr + 2, BYTES_PER_PIXEL, PixelIndices);
	wide_s32 OldGreens = GatherS32(BasePixelPtr + 1, BYTES_PER_PIXEL, PixelIndices);
	wide_s32 OldBlues  = GatherS32(BasePixelPtr + 0, BYTES_PER_PIXEL, PixelIndices);

	ConditionalAssign(&NewReds, ActivePixelMask, OldReds);
	ConditionalAssign(&NewGreens, ActivePixelMask, OldGreens);
	ConditionalAssign(&NewBlues, ActivePixelMask, OldBlues);

	alignas(4 * SIMD_WIDTH) static s32 R[SIMD_WIDTH];
	alignas(4 * SIMD_WIDTH) static s32 G[SIMD_WIDTH];
	alignas(4 * SIMD_WIDTH) static s32 B[SIMD_WIDTH];

#if (SIMD_WIDTH==1)
	R[0] = NewReds;
	G[0] = NewGreens;
	B[0] = NewBlues;
#elif (SIMD_WIDTH==4)
	_mm_store_si128((__m128i *)&R[0], NewReds.V);
	_mm_store_si128((__m128i *)&G[0], NewGreens.V);
	_mm_store_si128((__m128i *)&B[0], NewBlues.V);
#elif (SIMD_WIDTH==8)
	_mm256_store_si256((__m256i *)&R[0], NewReds.V);
	_mm256_store_si256((__m256i *)&G[0], NewGreens.V);
	_mm256_store_si256((__m256i *)&B[0], NewBlues.V);
#endif

	for (u32 LaneIdx = 0; LaneIdx < SIMD_WIDTH; LaneIdx += 1)
	{
		SetPixel(State->Bitmap, X + LaneIdx, Y,
			color_u8{u8(R[LaneIdx]), u8(G[LaneIdx]), u8(B[LaneIdx])});
	}
}

void
UpdateDepth(u32 *BaseDepthPtr,
		    wide_s32 ActivePixelMask,
		    wide_s32 OldDepth,
		    wide_s32 NewDepth)
{
	alignas(SIMD_WIDTH * 4) static u32 Depth[SIMD_WIDTH];

	ConditionalAssign(&NewDepth, ActivePixelMask, OldDepth);

#if (SIMD_WIDTH==1)
	Depth[0] = NewDepth;
#elif (SIMD_WIDTH==4)
	_mm_store_si128((__m128i *)&Depth[0], NewDepth.V);
#elif (SIMD_WIDTH==8)
	_mm256_store_si256((__m256i *)&Depth[0], NewDepth.V);
#endif

	CopyMemory(BaseDepthPtr, Depth, sizeof(Depth));
}

void
Draw(renderer_state *State,
	 u32 VertexCount)
{
	v3 *Vertices = (v3 *)State->VertexBuffer.Data;
	m4 WVP = State->WVP;
	u32 Stride = 2;

	for (u32 BaseID = 0; BaseID < VertexCount; BaseID += 3)
	{
		u32 Idx0 = Stride * (BaseID + 0);
		u32 Idx1 = Stride * (BaseID + 1);
		u32 Idx2 = Stride * (BaseID + 2);

		v3 Pos0 = Vertices[Idx0];
		v3 Pos1 = Vertices[Idx1];
		v3 Pos2 = Vertices[Idx2];
		v3 Color0 = Vertices[Idx0 + 1];
		v3 Color1 = Vertices[Idx1 + 1];
		v3 Color2 = Vertices[Idx2 + 1];

		v4 T0 = WVP * v4(Pos0.x, Pos0.y, Pos0.z, 1.0f);
		v4 T1 = WVP * v4(Pos1.x, Pos1.y, Pos1.z, 1.0f);
		v4 T2 = WVP * v4(Pos2.x, Pos2.y, Pos2.z, 1.0f);

		vertex ClippedVertices[12];

		ClippedVertices[0] = { T0, Color0 };
		ClippedVertices[1] = { T1, Color1 };
		ClippedVertices[2] = { T2, Color2 };

		vertex *ClippedVerticesEnd = ClipTriangle(ClippedVertices, ClippedVertices + 3);

		for (vertex *TriangleBegin = ClippedVertices;
			 TriangleBegin != ClippedVerticesEnd;
			 TriangleBegin += 3)
		{
			vertex V0 = TriangleBegin[0];
			vertex V1 = TriangleBegin[1];
			vertex V2 = TriangleBegin[2];

			triangle Triangle;

			Triangle.V0 = { PerspectiveDivide(V0.Pos), V0.Color };
			Triangle.V1 = { PerspectiveDivide(V1.Pos), V1.Color };
			Triangle.V2 = { PerspectiveDivide(V2.Pos), V2.Color };

			RasterizeTriangle(State, Triangle);
		}
	}
}

vertex
VertexShader(renderer_state *State,
   			 u32 VertexID)
{
	vertex		Output;
	v3			*Vertices = (v3 *)State->VertexBuffer.Data;
	m4			WVP = State->WVP;
	
	v3 Pos = Vertices[VertexID];

	Output.Pos = WVP * v4(Pos.x, Pos.y, Pos.z, 1.0f);
	Output.Color = Vertices[VertexID + 1];

	return (Output);
}

void
DrawIndexed(renderer_state *State,
			u32 IndexCount)
{
	v3 *Vertices = (v3 *)State->VertexBuffer.Data;
	u32 *Indices = (u32 *)State->IndexBuffer.Data;
	m4 WVP = State->WVP;
	u32 Stride = 2;

	for (u32 BaseID = 0; BaseID < IndexCount; BaseID += 3)
	{
		u32 Idx0 = Stride * Indices[BaseID + 0];
		u32 Idx1 = Stride * Indices[BaseID + 1];
		u32 Idx2 = Stride * Indices[BaseID + 2];

		vertex T0 = VertexShader(State, Idx0);
		vertex T1 = VertexShader(State, Idx1);
		vertex T2 = VertexShader(State, Idx2);

		vertex ClippedVertices[12];

		ClippedVertices[0] = T0;
		ClippedVertices[1] = T1;
		ClippedVertices[2] = T2;

		vertex *ClippedVerticesEnd = ClipTriangle(ClippedVertices, ClippedVertices + 3);

		for (vertex *TriangleBegin = ClippedVertices;
			 TriangleBegin != ClippedVerticesEnd;
			 TriangleBegin += 3)
		{
			vertex V0 = TriangleBegin[0];
			vertex V1 = TriangleBegin[1];
			vertex V2 = TriangleBegin[2];

			triangle Triangle;

			Triangle.V0 = { PerspectiveDivide(V0.Pos), V0.Color };
			Triangle.V1 = { PerspectiveDivide(V1.Pos), V1.Color };
			Triangle.V2 = { PerspectiveDivide(V2.Pos), V2.Color };

			RasterizeTriangle(State, Triangle);
		}
	}
}

buffer
CreateBuffer(void *Data,
			 u32 Size)
{
	buffer		Buffer;

	Buffer.Size = Size;
	Buffer.Data = HeapAlloc(GetProcessHeap(), 0, Size);
	CopyMemory(Buffer.Data, Data, Size);

	return (Buffer);
}

v4
PerspectiveDivide(v4 V)
{
	v4		Result = V;

	Result.w = 1.0f / V.w;
	Result.x *= Result.w;
	Result.y *= Result.w;
	Result.z *= Result.w;

	return (Result);
}

vertex
ClipIntersectEdge(vertex V0,
				  vertex V1,
    			  f32 Value0,
				  f32 Value1)
{
	vertex InterpolatedVertex;

	f32 t = Value0 / (Value0 - Value1);

	InterpolatedVertex.Pos = (1.0f - t) * V0.Pos + t * V1.Pos;
	InterpolatedVertex.Color = (1.0f - t) * V0.Color + t * V1.Color;

	return InterpolatedVertex;
}

vertex *
ClipTriangle(vertex *Triangle,
			 v4 Equation,
			 vertex *Result)
{
	f32 Values[3] =
	{
		Dot(Triangle[0].Pos, Equation),
		Dot(Triangle[1].Pos, Equation),
		Dot(Triangle[2].Pos, Equation),
	};

	u8 Mask = (Values[0] < 0.0f ? 0b001 : 0) |
			  (Values[1] < 0.0f ? 0b010 : 0) |
			  (Values[2] < 0.0f ? 0b100 : 0);

	switch (Mask)
	{
		// All vertices are inside allowed half-space
		// No clipping required, copy the triangle to output
		case 0b000:
		{
			*Result++ = Triangle[0];
			*Result++ = Triangle[1];
			*Result++ = Triangle[2];
		} break;

		// Vertex 0 is outside allowed half-space
		// Replace it with points on edges 01 and 02
		// And re-triangulate
		case 0b001:
		{
			vertex V01 = ClipIntersectEdge(Triangle[0], Triangle[1], Values[0], Values[1]);
			vertex V02 = ClipIntersectEdge(Triangle[0], Triangle[2], Values[0], Values[2]);
			*Result++ = V01;
			*Result++ = Triangle[1];
			*Result++ = Triangle[2];
			*Result++ = V01;
			*Result++ = Triangle[2];
			*Result++ = V02;
		} break;

		// Vertex 1 is outside allowed half-space
		// Replace it with points on edges 10 and 12
		// And re-triangulate
		case 0b010:
		{
			vertex V10 = ClipIntersectEdge(Triangle[1], Triangle[0], Values[1], Values[0]);
			vertex V12 = ClipIntersectEdge(Triangle[1], Triangle[2], Values[1], Values[2]);
			*Result++ = Triangle[0];
			*Result++ = V10;
			*Result++ = Triangle[2];
			*Result++ = Triangle[2];
			*Result++ = V10;
			*Result++ = V12;
		} break;

		// Vertices 0 and 1 are outside allowed half-space
		// Replace them with points on edges 02 and 12
		case 0b011:
		{
			*Result++ = ClipIntersectEdge(Triangle[0], Triangle[2], Values[0], Values[2]);
			*Result++ = ClipIntersectEdge(Triangle[1], Triangle[2], Values[1], Values[2]);
			*Result++ = Triangle[2];
		} break;

		// Vertex 2 is outside allowed half-space
		// Replace it with points on edges 20 and 21
		// And re-triangulate
		case 0b100:
		{
			vertex V20 = ClipIntersectEdge(Triangle[2], Triangle[0], Values[2], Values[0]);
			vertex V21 = ClipIntersectEdge(Triangle[2], Triangle[1], Values[2], Values[1]);
			*Result++ = Triangle[0];
			*Result++ = Triangle[1];
			*Result++ = V20;
			*Result++ = V20;
			*Result++ = Triangle[1];
			*Result++ = V21;
		} break;

		// Vertices 0 and 2 are outside allowed half-space
		// Replace them with points on edges 01 and 21
		case 0b101:
		{
			*Result++ = ClipIntersectEdge(Triangle[0], Triangle[1], Values[0], Values[1]);
			*Result++ = Triangle[1];
			*Result++ = ClipIntersectEdge(Triangle[2], Triangle[1], Values[2], Values[1]);
		} break;

		// Vertices 1 and 2 are outside allowed half-space
		// Replace them with points on edges 10 and 20
		case 0b110:
		{
			*Result++ = Triangle[0];
			*Result++ = ClipIntersectEdge(Triangle[1], Triangle[0], Values[1], Values[0]);
			*Result++ = ClipIntersectEdge(Triangle[2], Triangle[0], Values[2], Values[0]);
		} break;

		// All vertices are outside allowed half-space
		// Clip the whole Triangle, result is empty
		case 0b111:
		{
		} break;
	}

	return Result;
}

vertex *
ClipTriangle(vertex *Begin,
			 vertex *End)
{
	static const v4 Equations[2] =
	{
		v4(0, 0, 1, 1),	// Z > -W
		v4(0, 0, -1, 1)	// Z < W
	};

	vertex Results[12];

	for (auto Equation : Equations)
	{
		vertex *ResultEnd = Results;

		for (vertex *Triangle = Begin; Triangle != End; Triangle += 3)
		{
			ResultEnd = ClipTriangle(Triangle, Equation, ResultEnd);
		}

		End = std::copy(Results, ResultEnd, Begin);
	}

	return End;
}

texture				
CreateTexture(const char *Filename)
{
	texture		Texture;
	s32			NumComponents;
	u8			*PixelData;

	PixelData = stbi_load(Filename, &Texture.Width, &Texture.Height, &NumComponents, 4);

	assert(PixelData != NULL);

	Texture.Data = PixelData;

	return (Texture);
}

