/*
   NOTE(matthew): Completed
   - Edge fill-rule
   - Subpixel precision
   - Incremental edge function computation
   - 4-wide SIMD (SSE)
   - Full transform (WVP + perspective + 1/z) pipeline

   TODO(matthew):
   - 8-wide SIMD (AVX)
   - Depth buffering
   - Perspective-correct interpolation
   - Clipping
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define MG_IMPL
#include <mg.h>
#include <bitmap.h>
#include <fixed_point.h>
#include <simd.h>

#define SCR_WIDTH 	1024
#define SCR_HEIGHT 	768

struct triangle
{
	v2		V0,
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
	buffer		VertexBuffer;
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

LRESULT CALLBACK	WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
s32_fp 				Orient2D(v2_fp A, v2_fp B, v2_fp C);
v2					NdcToRaster(v2 Point);
b32  				FillRule(v2_fp Edge);
void 				RasterizeTriangle(bitmap *Bitmap, triangle Triangle);
void				SetPixels_4x(bitmap *Bitmap, s32 X, s32 Y, wide_s32 ActivePixelMask, weights Weights);
v4					PerspectiveDivide(v4 V);
void				Draw(renderer_state *State, u32 Count);
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

	v3		Vertices[] =
	{
		v3(-0.5f, -0.5f, 0.5f),
		v3( 0.5f, -0.5f, 0.5f),
		v3( 0.5f,  0.5f, 0.5f),

		v3(-0.5f, -0.5f, 0.5f),
		v3( 0.5f,  0.5f, 0.5f),
		v3(-0.5f,  0.5f, 0.5f),
	};

	buffer VertexBuffer = CreateBuffer(Vertices, sizeof(Vertices));

	///////////////////////////////////
	// Main loop

	MSG					Message;
	f32 				Angle = 0;
	LARGE_INTEGER		Start, End, Frequency;
	f32					Freq;
	renderer_state		State;
	camera				Camera;


	QueryPerformanceFrequency(&Frequency);
	Freq = Frequency.QuadPart / 1000.0f;

	State.VertexBuffer = VertexBuffer;
	State.Bitmap = &Bitmap;

	Camera.Pos = v3(0, 0, -2);
	Camera.Front = v3(0, 0, 1);
	Camera.Up = v3(0, 1, 0);

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

			m4 World = Mat4Rotate(Angle, v3(0, 0, 1));//Mat4Identity();
			m4 View = Mat4LookAtLH(Camera.Pos, Camera.Front, Camera.Up);
			m4 Proj = Mat4PerspectiveLH(45.0f, f32(SCR_WIDTH) / f32(SCR_HEIGHT), 0.1f, 1000.0f);

			State.WVP = Proj * View * World;

			Draw(&State, 6);

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

	v2 RasterV0 = NdcToRaster(Triangle.V0);
	v2 RasterV1 = NdcToRaster(Triangle.V1);
	v2 RasterV2 = NdcToRaster(Triangle.V2);

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
				wide_s32 Sum = (W0 + W1 + W2) >> FP_SHIFT;
				weights Weights;

				wide_s32 MaskedW0 = W0 & Comparison;
				wide_s32 MaskedW1 = W1 & Comparison;
				wide_s32 MaskedW2 = W2 & Comparison;

				Weights.W0 = WideF32FromS32(MaskedW0 >> FP_SHIFT) / WideF32FromS32(Sum);
				Weights.W1 = WideF32FromS32(MaskedW1 >> FP_SHIFT) / WideF32FromS32(Sum);
				Weights.W2 = WideF32FromS32(MaskedW2 >> FP_SHIFT) / WideF32FromS32(Sum);

				SetPixels_4x(Bitmap, X, Y, Comparison, Weights);
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
			 weights Weights)
{
	wide_f32 Wide256 = wide_f32(255.999f);

	wide_f32 Reds = Wide256 * Weights.W0;
	wide_f32 Greens = Wide256 * Weights.W1;
	wide_f32 Blues = Wide256 * Weights.W2;

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
	 u32 Count)
{
	v3 *Vertices = (v3 *)State->VertexBuffer.Data;
	m4 WVP = State->WVP;

	for (u32 BaseID = 0; BaseID < Count; BaseID += 3)
	{
		v3 V0 = Vertices[BaseID + 0];
		v3 V1 = Vertices[BaseID + 1];
		v3 V2 = Vertices[BaseID + 2];

		v4 T0 = WVP * v4(V0.x, V0.y, V0.z, 1.0f);
		v4 T1 = WVP * v4(V1.x, V1.y, V1.z, 1.0f);
		v4 T2 = WVP * v4(V2.x, V2.y, V2.z, 1.0f);

		T0 = PerspectiveDivide(T0);
		T1 = PerspectiveDivide(T1);
		T2 = PerspectiveDivide(T2);

		triangle Triangle;

		Triangle.V0 = v2(T0.x, T0.y);
		Triangle.V1 = v2(T1.x, T1.y);
		Triangle.V2 = v2(T2.x, T2.y);

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

