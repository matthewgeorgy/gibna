#define MG_IMPL
#include <mg.h>
#include <stdio.h>

#define SCR_WIDTH 	1024
#define SCR_HEIGHT 	768
#define BYTES_PER_PIXEL		4
#define BITMAP_MEMORY_SIZE	(SCR_WIDTH * SCR_HEIGHT * BYTES_PER_PIXEL)

struct bitmap
{
	BITMAPINFO	Info;
	u8			*Memory;
	s32 		Width,
				Height;
	HWND 		Window;
	HDC			DC;
};

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

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


	Bitmap.Width = SCR_WIDTH;
	Bitmap.Height = SCR_HEIGHT;
	Bitmap.Window = Window;
	Bitmap.DC = GetDC(Window);
	Bitmap.Memory = (u8 *)HeapAlloc(GetProcessHeap(), 0, BITMAP_MEMORY_SIZE);

	Bitmap.Info.bmiHeader.biSize = sizeof(Bitmap.Info.bmiHeader);
	Bitmap.Info.bmiHeader.biWidth = Bitmap.Width;
	Bitmap.Info.bmiHeader.biHeight = Bitmap.Height; // NOTE(matthew): bottom->up bitmap
	Bitmap.Info.bmiHeader.biPlanes = 1;
	Bitmap.Info.bmiHeader.biBitCount = 32;
	Bitmap.Info.bmiHeader.biCompression = BI_RGB;

	for (s32 Y = 0; Y < Bitmap.Height; Y += 1)
	{
		for (s32 X = 0; X < Bitmap.Width; X += 1)
		{
			s32 PixelCoord = (X + Y * Bitmap.Width) * BYTES_PER_PIXEL;

			Bitmap.Memory[PixelCoord + 0] = 255;
			Bitmap.Memory[PixelCoord + 1] = 0;
			Bitmap.Memory[PixelCoord + 2] = 0;
			Bitmap.Memory[PixelCoord + 3] = 0;
		}
	}

	StretchDIBits(Bitmap.DC,
			0, 0, Bitmap.Width, Bitmap.Height,
			0, 0, Bitmap.Width, Bitmap.Height,
			Bitmap.Memory, &Bitmap.Info, DIB_RGB_COLORS, SRCCOPY);

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

