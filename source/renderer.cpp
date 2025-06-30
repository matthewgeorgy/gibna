#include <renderer.h>

void
RasterizeTriangle(bitmap *Bitmap,
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
	MinX = (s32)(Min(Min(V0.x, V1.x), V2.x)) >> FP_SHIFT;
	MaxX = (s32)(Max(Max(V0.x, V1.x), V2.x)) >> FP_SHIFT;
	MinY = (s32)(Min(Min(V0.y, V1.y), V2.y)) >> FP_SHIFT;
	MaxY = (s32)(Max(Max(V0.y, V1.y), V2.y)) >> FP_SHIFT;

	// Screen clipping
	MinX = Max(MinX, 0);
	MaxX = Min(MaxX, SCR_WIDTH);
	MinY = Max(MinY, 0);
	MaxY = Min(MaxY, SCR_HEIGHT);

	// Initial edge function values
	v2_fp Pixel = v2_fp(f32(MinX) + 0.5f, f32(MinY) + 0.5f);
	edge E01, E12, E20;
	wide_s32 WideOne = wide_s32(1);
	wide_s32 WideZero = wide_s32(0);

	wide_s32 W0Row = E12.Init(V1, V2, Pixel);
	wide_s32 W1Row = E20.Init(V2, V0, Pixel);
	wide_s32 W2Row = E01.Init(V0, V1, Pixel);

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
				wide_f32 L0 = WideF32FromS32(W0 >> FP_SHIFT) * Triangle.V0.Pos.w;
				wide_f32 L1 = WideF32FromS32(W1 >> FP_SHIFT) * Triangle.V1.Pos.w;
				wide_f32 L2 = WideF32FromS32(W2 >> FP_SHIFT) * Triangle.V2.Pos.w;
				wide_f32 Sum = L0 + L1 + L2;

				weights Weights;

				Weights.W0 = L0 / Sum;
				Weights.W1 = L1 / Sum;
				Weights.W2 = L2 / Sum;

				// Depth
				wide_f32 Z = Weights.W0 * Triangle.V0.Pos.z + 
							 Weights.W1 * Triangle.V1.Pos.z +
							 Weights.W2 * Triangle.V2.Pos.z;
				/* wide_f32 MaxDepthValue = WideF32FromS32(wide_s32(0x7FFFFFFF)); */
				/* wide_s32 NewDepth = WideS32FromF32((wide_f32(0.5f) + wide_f32(0.5f) * Z) * MaxDepthValue); */
				wide_f32 Step1 = wide_f32(0.5f) * Z;
				wide_f32 Step2 = wide_f32(0.5f) + Step1;
				wide_f32 Step3 = WideF32FromS32(wide_s32(0x7FFFFFFF));
				wide_s32 NewDepth = WideS32FromF32(Step2 * Step3);

				s32 DepthPixelCoord = X + Y * Bitmap->Width;
				u32 *BaseDepthPtr = &Bitmap->DepthBuffer[DepthPixelCoord];
				wide_s32 OldDepth = GatherS32(BaseDepthPtr, sizeof(u32), WIDE_S32_ZERO_TO_RANGE);

				wide_s32 DepthMask = NewDepth < OldDepth;
				wide_s32 ActivePixelMask = ColorMask & DepthMask;

				if (AnyTrue(ActivePixelMask))
				{
					color_triple Colors;

					Colors.C0 = Triangle.V0.Color;
					Colors.C1 = Triangle.V1.Color;
					Colors.C2 = Triangle.V2.Color;

					SetPixels_4x(Bitmap, X, Y, ActivePixelMask, Weights, Colors);

					alignas(16) static u32 Depth[4];
					ConditionalAssign(&NewDepth, ActivePixelMask, OldDepth);
					_mm_store_si128((__m128i *)&Depth[0], NewDepth.V);
					CopyMemory(BaseDepthPtr, Depth, sizeof(Depth));
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
	s32 A = (V0.y - V1.y);
	s32 B = (V1.x - V0.x);
	s32 C = (V0.x * V1.y) - (V0.y * V1.x);

	this->OneStepX = wide_s32((A << FP_SHIFT) * StepSizeX);
	this->OneStepY = wide_s32((B << FP_SHIFT) * StepSizeY);

	wide_s32 X = wide_s32(P.x) + WIDE_S32_ZERO_TO_RANGE * wide_s32(FP_MULTIPLIER);
	wide_s32 Y = wide_s32(P.y);

	return (wide_s32(A) * X + wide_s32(B) * Y + wide_s32(C));
}

void
SetPixels_4x(bitmap *Bitmap,
			 s32 X, 
			 s32 Y, 
			 wide_s32 ActivePixelMask,
			 weights Weights,
			 color_triple Colors)
{
	wide_f32 Wide256 = wide_f32(255.999f);

	wide_v3 NewColor0 = wide_v3(Colors.C0.r * Weights.W0,
								Colors.C0.g * Weights.W0,
								Colors.C0.b * Weights.W0);
	wide_v3 NewColor1 = wide_v3(Colors.C1.r * Weights.W1,
								Colors.C1.g * Weights.W1,
								Colors.C1.b * Weights.W1);
	wide_v3 NewColor2 = wide_v3(Colors.C2.r * Weights.W2,
								Colors.C2.g * Weights.W2,
								Colors.C2.b * Weights.W2);

	NewColor0.r = Colors.C0.r * Weights.W0;

	wide_v3 NewColor = NewColor0 + NewColor1 + NewColor2;

	NewColor.r = NewColor.r * Wide256;
	NewColor.g = NewColor.g * Wide256;
	NewColor.b = NewColor.b * Wide256;

	wide_f32 Reds = NewColor.r;
	wide_f32 Greens = NewColor.g;
	wide_f32 Blues = NewColor.b;

	wide_s32 NewReds = WideS32FromF32(Reds);
	wide_s32 NewGreens = WideS32FromF32(Greens);
	wide_s32 NewBlues = WideS32FromF32(Blues);

	wide_s32 PixelIndices = WIDE_S32_ZERO_TO_RANGE;
	s32 PixelCoord = (X + Y * Bitmap->Width) * BYTES_PER_PIXEL;
	u8 *BasePixelPtr = &Bitmap->ColorBuffer[PixelCoord];

	wide_s32 OldReds   = GatherS32(BasePixelPtr + 2, BYTES_PER_PIXEL, PixelIndices);
	wide_s32 OldGreens = GatherS32(BasePixelPtr + 1, BYTES_PER_PIXEL, PixelIndices);
	wide_s32 OldBlues  = GatherS32(BasePixelPtr + 0, BYTES_PER_PIXEL, PixelIndices);

	ConditionalAssign(&NewReds, ActivePixelMask, OldReds);
	ConditionalAssign(&NewGreens, ActivePixelMask, OldGreens);
	ConditionalAssign(&NewBlues, ActivePixelMask, OldBlues);

	alignas(16) static s32 R[4];
	alignas(16) static s32 G[4];
	alignas(16) static s32 B[4];

	_mm_store_si128((__m128i *)&R[0], NewReds.V);
	_mm_store_si128((__m128i *)&G[0], NewGreens.V);
	_mm_store_si128((__m128i *)&B[0], NewBlues.V);

	SetPixel(Bitmap, X + 0, Y, color_u8{u8(R[0]), u8(G[0]), u8(B[0])});
	SetPixel(Bitmap, X + 1, Y, color_u8{u8(R[1]), u8(G[1]), u8(B[1])});
	SetPixel(Bitmap, X + 2, Y, color_u8{u8(R[2]), u8(G[2]), u8(B[2])});
	SetPixel(Bitmap, X + 3, Y, color_u8{u8(R[3]), u8(G[3]), u8(B[3])});
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

		v3 V0 = Vertices[Idx0];
		v3 V1 = Vertices[Idx1];
		v3 V2 = Vertices[Idx2];

		v4 T0 = WVP * v4(V0.x, V0.y, V0.z, 1.0f);
		v4 T1 = WVP * v4(V1.x, V1.y, V1.z, 1.0f);
		v4 T2 = WVP * v4(V2.x, V2.y, V2.z, 1.0f);

		triangle Triangle;

		Triangle.V0 = { PerspectiveDivide(T0), Vertices[Idx0 + 1] };
		Triangle.V1 = { PerspectiveDivide(T1), Vertices[Idx1 + 1] };
		Triangle.V2 = { PerspectiveDivide(T2), Vertices[Idx2 + 1] };

		RasterizeTriangle(State->Bitmap, Triangle);
	}
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

		v3 V0 = Vertices[Idx0];
		v3 V1 = Vertices[Idx1];
		v3 V2 = Vertices[Idx2];

		v4 T0 = WVP * v4(V0.x, V0.y, V0.z, 1.0f);
		v4 T1 = WVP * v4(V1.x, V1.y, V1.z, 1.0f);
		v4 T2 = WVP * v4(V2.x, V2.y, V2.z, 1.0f);

		triangle Triangle;

		Triangle.V0 = { PerspectiveDivide(T0), Vertices[Idx0 + 1] };
		Triangle.V1 = { PerspectiveDivide(T1), Vertices[Idx1 + 1] };
		Triangle.V2 = { PerspectiveDivide(T2), Vertices[Idx2 + 1] };

		RasterizeTriangle(State->Bitmap, Triangle);
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
