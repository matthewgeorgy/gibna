/*
   NOTE(matthew): Completed
   - Edge fill-rule
   - Subpixel precision
   - Incremental edge function computation
   - 4-wide SIMD (SSE)
   - Full transform (WVP + perspective + 1/z) pipeline
   - Perspective-correct interpolation
   - Depth buffering
   - Clipping

   TODO(matthew):
   - 8-wide SIMD (AVX)
   - Fix edge artifacts
   - Mesh loading
   - Textures
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define MG_IMPL
#include <mg.h>
#include <bitmap.h>
#include <fixed_point.h>
#include <simd.h>
#include <renderer.h>

LRESULT CALLBACK	WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

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
			World = Mat4Rotate(Angle, v3(0, 1, 0)) * Mat4Translate(0, 0, -12.5f);
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

