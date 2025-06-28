/*
   NOTE(matthew): Completed
   - Edge fill-rule
   - Subpixel precision

   TODO(matthew):
   - Incremental edge function computation
   - 4-wide SIMD (SSE)
   - 8-wide SIMD (AVX)
   - Full transform (WVP + perspective + 1/z) pipeline
   - Depth buffering
   - Perspective-correct interpolation
   - Clipping
*/

#include <stdio.h>

#define MG_IMPL
#include <mg.h>
#include <bitmap.h>
#include <fixed_point.h>

#define SCR_WIDTH 	1024
#define SCR_HEIGHT 	768

struct triangle
{
	v2		V0,
			V1,
			V2;
};

LRESULT CALLBACK	WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
s32_fp 				Orient2D(v2_fp A, v2_fp B, v2_fp C);
v2					NdcToRaster(v2 Point);
b32  				FillRule(v2_fp Edge);
void 				RasterizeTriangle(bitmap *Bitmap, triangle Triangle);

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

	v2 V0 = v2(-0.5f, -0.5f);
	v2 V1 = v2( 0.5f, -0.5f);
	v2 V2 = v2( 0.0f,  0.5f);

	///////////////////////////////////
	// Main loop

	MSG		Message;
	f32 	Angle = 0;


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

			f32 CosAngle = Cos(Angle);
			f32 SinAngle = Sin(Angle);

			triangle Triangle;

			Triangle.V0.x = V0.x * CosAngle - V0.y * SinAngle;
			Triangle.V0.y = V0.x * SinAngle + V0.y * CosAngle;
			Triangle.V1.x = V1.x * CosAngle - V1.y * SinAngle;
			Triangle.V1.y = V1.x * SinAngle + V1.y * CosAngle;
			Triangle.V2.x = V2.x * CosAngle - V2.y * SinAngle;
			Triangle.V2.y = V2.x * SinAngle + V2.y * CosAngle;

			RasterizeTriangle(&Bitmap, Triangle);

			PresentBitmap(Bitmap);

			Angle += 0.005f;
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

	for (s32 Y = MinY; Y < MaxY; Y += 1)
	{
		for (s32 X = MinX; X < MaxX; X += 1)
		{
			v2_fp Pixel = v2_fp(f32(X) + 0.5f, f32(Y) + 0.5f);

			s32_fp W0 = Orient2D(V1, V2, Pixel);
			s32_fp W1 = Orient2D(V2, V0, Pixel);
			s32_fp W2 = Orient2D(V0, V1, Pixel);

			if (FillRule(V2 - V1))	W0 -= 1;
			if (FillRule(V0 - V2))	W1 -= 1;
			if (FillRule(V1 - V0))	W2 -= 1;

			if ((W0 | W1 | W2) >= 0)
			{
				color_u8 Color = { 0, 255, 0 };

				SetPixel(Bitmap, X, Y, Color);
			}
		}
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

