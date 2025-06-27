/*
   TODO(matthew):
   - Edge fill-rule
   - Incremental edge function computation
   - 4-wide SIMD (SSE)
   - 8-wide SIMD (AVX)
   - Full transform (WVP + perspective + 1/z) pipeline
   - Subpixel precision
   - Depth buffering
   - Perspective-correct interpolation
   - Clipping
*/

#include <stdio.h>

#define MG_IMPL
#include <mg.h>
#include <bitmap.h>

#define SCR_WIDTH 	1024
#define SCR_HEIGHT 	768

struct triangle
{
	v2		V0,
			V1,
			V2;
};

LRESULT CALLBACK	WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
s32 				Orient2D(v2i A, v2i B, v2i C);
v2i					NdcToRaster(v2 Point);

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
	// Triangle

	s32				MinX, MaxX,
					MinY, MaxY;
	triangle		Triangle;

	// Vertices
	Triangle.V0 = v2(-0.5f, -0.5f);
	Triangle.V1 = v2( 0.5f, -0.5f);
	Triangle.V2 = v2( 0.0f,  0.5f);

	v2i V0 = NdcToRaster(Triangle.V0);
	v2i V1 = NdcToRaster(Triangle.V1);
	v2i V2 = NdcToRaster(Triangle.V2);

	// Triangle bounding box
	MinX = Min(Min(V0.x, V1.x), V2.x);
	MaxX = Max(Max(V0.x, V1.x), V2.x);
	MinY = Min(Min(V0.y, V1.y), V2.y);
	MaxY = Max(Max(V0.y, V1.y), V2.y);

	// Screen clipping
	MinX = Max(MinX, 0);
	MaxX = Min(MaxX, SCR_WIDTH);
	MinY = Max(MinY, 0);
	MaxY = Min(MaxY, SCR_HEIGHT);

	for (s32 Y = MinY; Y < MaxY; Y += 1)
	{
		for (s32 X = MinX; X < MaxX; X += 1)
		{
			v2i Pixel = v2i(X, Y);

			s32 W0 = Orient2D(V1, V2, Pixel);
			s32 W1 = Orient2D(V2, V0, Pixel);
			s32 W2 = Orient2D(V0, V1, Pixel);

			if ((W0 | W1 | W2) >= 0)
			{
				s32 PixelCoord = (X + Y * Bitmap.Width) * BYTES_PER_PIXEL;

				Bitmap.Memory[PixelCoord + 0] = 0;
				Bitmap.Memory[PixelCoord + 1] = 255;
				Bitmap.Memory[PixelCoord + 2] = 0;
				Bitmap.Memory[PixelCoord + 3] = 0;
			}
		}
	}

	PresentBitmap(Bitmap);

	///////////////////////////////////
	// Main loop

	MSG		Message;


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
		}
	}

	return 0;
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

s32
Orient2D(v2i A,
		 v2i B,
		 v2i C)
{
	s32		Det;


	Det = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);

	return (Det);
}

v2i
NdcToRaster(v2 Point)
{
	v2i		Pixel;


	Pixel.x = s32((Point.x + 1.0f) * 0.5 * SCR_WIDTH);
	Pixel.y = s32((Point.y + 1.0f) * 0.5 * SCR_HEIGHT);

	return (Pixel);
}

