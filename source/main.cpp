/*
   NOTE(matthew): Completed
   - Edge fill-rule
   - Subpixel precision
   - Incremental edge function computation
   - 4-wide SIMD (SSE)
   - Full transform (WVP + perspective + 1/z) pipeline
   - Perspective-correct interpolation

   TODO(matthew):
   - 8-wide SIMD (AVX)
   - Depth buffering
   - Clipping
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define MG_IMPL
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

LRESULT CALLBACK	WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
s32_fp 				Orient2D(v2_fp A, v2_fp B, v2_fp C);
v2					NdcToRaster(v2 Point);
b32  				FillRule(v2_fp Edge);
void 				RasterizeTriangle(bitmap *Bitmap, triangle Triangle);
void				SetPixels_4x(bitmap *Bitmap, s32 X, s32 Y, wide_s32 ActivePixelMask, weights Weights, color_triple Colors);
v4					PerspectiveDivide(v4 V);
void				Draw(renderer_state *State, u32 VertexCount);
void				DrawIndexed(renderer_state *State, u32 IndexCount);
buffer 				CreateBuffer(void *Data, u32 Size);

int
main(void)
{
	///////////////////////////////////
	// Win32

	WNDCLASSEX		WndClass = {};
	HWND			Window;
	ATOM			Atom;
	RECT 			WindowDim = {};
	u32 			WindowWidth,
					WindowHeight;


	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	WndClass.lpszClassName = "FromageClassName";
	WndClass.lpfnWndProc = &WndProc;
	WndClass.hInstance = GetModuleHandle(NULL);

	Atom = RegisterClassEx(&WndClass);
	if (!Atom)
	{
		printf("Failed to register class...!\r\n");
		return (-1);
	}

	WindowDim.right = SCR_WIDTH;
	WindowDim.bottom = SCR_HEIGHT;
	AdjustWindowRect(&WindowDim, WS_OVERLAPPEDWINDOW, FALSE);
	WindowWidth = WindowDim.right - WindowDim.left;
	WindowHeight = WindowDim.bottom - WindowDim.top;

	Window = CreateWindowEx(0, WndClass.lpszClassName, "fromage",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		WindowWidth, WindowHeight,
		NULL, NULL, WndClass.hInstance, NULL);

	if (!Window)
	{
		printf("Failed to create window...!\r\n");
		return (-1);
	}

	ShowWindow(Window, SW_SHOW);
	UpdateWindow(Window);

	///////////////////////////////////
	// Bitmap

	bitmap Bitmap;


	AllocateBitmap(&Bitmap, Window, SCR_WIDTH, SCR_HEIGHT);

	///////////////////////////////////
	// Vertices

	f32 	Vertices[] =
	{
		-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	1.0f, 1.0f, 1.0f,
	};
	u32		Indices[] =
	{
		// front face
		2, 1, 0,
		3, 2, 0,

		// back face
		5, 6, 4,
		6, 7, 4,

		// left face
		1, 5, 4,
		0, 1, 4,

		// right face
		6, 2, 3,
		7, 6, 3,

		// top face
		6, 5, 1,
		2, 6, 1,

		// bottom face
		3, 0, 4,
		7, 3, 4,
	};

	buffer VertexBuffer = CreateBuffer(Vertices, sizeof(Vertices));
	buffer IndexBuffer = CreateBuffer(Indices, sizeof(Indices));

	///////////////////////////////////
	// Main loop

	MSG					Message;
	f32 				Angle = 0;
	LARGE_INTEGER		Start, End, Frequency;
	f32					Freq;
	renderer_state		State;
	camera				Camera;
	m4 					World,
						View,
						Proj;


	QueryPerformanceFrequency(&Frequency);
	Freq = Frequency.QuadPart / 1000.0f;

	State.VertexBuffer = VertexBuffer;
	State.IndexBuffer = IndexBuffer;
	State.Bitmap = &Bitmap;

	Camera.Pos = v3(0, 3, -8);
	Camera.Front = v3(0, 0, 0);
	Camera.Up = v3(0, 1, 0);

	View = Mat4LookAtLH(Camera.Pos, Camera.Front, Camera.Up);
	Proj = Mat4PerspectiveLH(45.0f, f32(SCR_WIDTH) / f32(SCR_HEIGHT), 0.1f, 1000.0f);

	for (;;)
	{
		if (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
		{
			ClearBitmap(&Bitmap);

			QueryPerformanceCounter(&Start);

			// Cube 1
			World = Mat4Rotate(Angle, v3(0, 1, 0)) * Mat4Translate(0, 0, 2.5f);
			State.WVP = Proj * View * World;
			DrawIndexed(&State, _countof(Indices));

			// Cube 2
			World = Mat4Rotate(-Angle, v3(0, 1, 0)) * Mat4Scale(1.3f);
			State.WVP = Proj * View * World;
			DrawIndexed(&State, _countof(Indices));

			QueryPerformanceCounter(&End);

			PresentBitmap(Bitmap);

			static CHAR Buffer[256];
			sprintf(Buffer, "gibna --- %f ms / frame", (End.QuadPart - Start.QuadPart) / Freq);
			SetWindowText(Window, Buffer);

			Angle += 1.0f;
		}
	}

	return 0;
}

void
RasterizeTriangle(bitmap *Bitmap,
				  triangle Triangle)
{
	s32				MinX, MaxX,
					MinY, MaxY;

	v2 RasterV0 = NdcToRaster(v2(Triangle.V0.x, Triangle.V0.y));
	v2 RasterV1 = NdcToRaster(v2(Triangle.V1.x, Triangle.V1.y));
	v2 RasterV2 = NdcToRaster(v2(Triangle.V2.x, Triangle.V2.y));

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
			wide_s32 Mask = W0 | W1 | W2;
			wide_s32 Comparison = Mask >= WideZero;

			if (AnyTrue(Comparison))
			{
				wide_f32 L0 = WideF32FromS32(W0 >> FP_SHIFT) * Triangle.V0.w;
				wide_f32 L1 = WideF32FromS32(W1 >> FP_SHIFT) * Triangle.V1.w;
				wide_f32 L2 = WideF32FromS32(W2 >> FP_SHIFT) * Triangle.V2.w;
				wide_f32 Sum = L0 + L1 + L2;

				weights Weights;

				Weights.W0 = L0 / Sum;
				Weights.W1 = L1 / Sum;
				Weights.W2 = L2 / Sum;

				color_triple Colors;

				Colors.C0 = Triangle.Color0;
				Colors.C1 = Triangle.Color1;
				Colors.C2 = Triangle.Color2;

				SetPixels_4x(Bitmap, X, Y, Comparison, Weights, Colors);
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

LRESULT CALLBACK
WndProc(HWND hWnd,
		UINT Msg,
		WPARAM wParam,
		LPARAM lParam)
{
	LRESULT 	Result = 0;

	switch (Msg)
	{
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
		} break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);
		} break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;

		default:
		{
			Result = DefWindowProcA(hWnd, Msg, wParam, lParam);
		} break;
	}

	return (Result);
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
	u8 *BasePixelPtr = &Bitmap->Memory[PixelCoord];

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

		Triangle.V0 = PerspectiveDivide(T0);
		Triangle.V1 = PerspectiveDivide(T1);
		Triangle.V2 = PerspectiveDivide(T2);

		Triangle.Color0 = Vertices[Idx0 + 1];
		Triangle.Color1 = Vertices[Idx1 + 1];
		Triangle.Color2 = Vertices[Idx2 + 1];

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

		Triangle.V0 = PerspectiveDivide(T0);
		Triangle.V1 = PerspectiveDivide(T1);
		Triangle.V2 = PerspectiveDivide(T2);

		Triangle.Color0 = Vertices[Idx0 + 1];
		Triangle.Color1 = Vertices[Idx1 + 1];
		Triangle.Color2 = Vertices[Idx2 + 1];

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

