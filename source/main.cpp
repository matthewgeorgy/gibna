/*
   NOTE(matthew): Completed
   - Edge fill-rule
   - Subpixel precision
   - Incremental edge function computation
   - 4-wide SIMD (SSE)
   - 8-wide SIMD (AVX)
   - Full transform (WVP + perspective + 1/z) pipeline
   - Perspective-correct interpolation
   - Depth buffering
   - Clipping
   - Textures

   TODO(matthew):
   - Improve mesh loading; more types of OBJs (and PLYs too???)
   - "Shaders" / rendering API
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define MG_IMPL
#include <mg.h>
#include <bitmap.h>
#include <fixed_point.h>
#include <simd.h>
#include <renderer.h>
#include <mesh.h>

LRESULT CALLBACK	WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void				GenerateSphere(array<f32> *Vertices, array<u32> *Indices);

vertex				SphereVS(renderer_state *State, u32 VertexID);
wide_v3				SpherePS(renderer_state *State, vertex_attribs Attribs);

vertex				CubeVS(renderer_state *State, u32 VertexID);
wide_v3				CubePS(renderer_state *State, vertex_attribs Attribs);

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
	WndClass.lpszClassName = "GibnaClassName";
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

	Window = CreateWindowEx(0, WndClass.lpszClassName, "gibna",
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

	array<f32>		SphereVertices;
	array<u32>		SphereIndices;
	mesh			CubeMesh;


	GenerateSphere(&SphereVertices, &SphereIndices);
	LoadMesh(&CubeMesh, "assets/cube.obj");

	buffer SphereVB = CreateBuffer(SphereVertices.Data, SphereVertices.ByteSize());
	buffer SphereIB = CreateBuffer(SphereIndices.Data, SphereIndices.ByteSize());

	buffer CubeVB = CreateBuffer(CubeMesh.Vertices.Data, CubeMesh.Vertices.ByteSize());

	///////////////////////////////////
	// Texture

	texture		Texture;


	Texture = CreateTexture("assets/earth.jpg");

	///////////////////////////////////
	// Timers

	LARGE_INTEGER		StartingTime, Frequency;
	s64					UpdateTitle;
	f32					TicksPerMillisecond;
	u32					FrameCount = 0;
	s64					TotalRenderingTime = 0;


	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	UpdateTitle = StartingTime.QuadPart + Frequency.QuadPart / 2;
	TicksPerMillisecond = Frequency.QuadPart / 1000.0f;

	///////////////////////////////////
	// Main loop

	MSG					Message;
	f32 				Angle = 0;
	renderer_state		State;
	camera				Camera;
	m4 					World,
						View,
						Proj;


	State.Bitmap = &Bitmap;
	State.Texture = Texture;

	Camera.Pos = v3(0, 2, -6);
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

			LARGE_INTEGER Start, End;
			QueryPerformanceCounter(&Start);

			// Sphere render
			State.VertexBuffer = SphereVB;
			State.IndexBuffer = SphereIB;
			State.VS = SphereVS;
			State.PS = SpherePS;

			World = Mat4Rotate(Angle, v3(0, 1, 0)) * Mat4Rotate(-90, v3(1, 0, 0));
			State.WVP = Proj * View * World;
			DrawIndexed(&State, SphereIndices.Len());

			// Cube render
			State.VertexBuffer = CubeVB;
			State.VS = CubeVS;
			State.PS = CubePS;

			World = Mat4Scale(0.5f) * Mat4Rotate(-Angle, v3(0, 1, 0)) * Mat4Translate(0, 0, 5.0f);
			State.WVP = Proj * View * World;
			Draw(&State, CubeMesh.Vertices.Len() / 2);

			PresentBitmap(Bitmap);
			Angle += 0.1f;
			FrameCount += 1;

			QueryPerformanceCounter(&End);

			TotalRenderingTime += (End.QuadPart - Start.QuadPart);

			if (End.QuadPart >= UpdateTitle)
			{
				UpdateTitle = End.QuadPart + Frequency.QuadPart / 2;
				f32 MillisecondsPerFrame = TotalRenderingTime / (f32(FrameCount) * TicksPerMillisecond);
				FrameCount = 0;
				TotalRenderingTime = 0;

				static CHAR Title[256];
				sprintf(Title, "gibna (%d-wide) --- %f ms / frame", SIMD_WIDTH, MillisecondsPerFrame);
				SetWindowText(Window, Title);
			}
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

void				
GenerateSphere(array<f32> *Vertices,
			   array<u32> *Indices)
{
	s32 SectorCount = 36;
	s32 StackCount = 18;
	f32 SectorStep = (2 * PI) / f32(SectorCount);
	f32 StackStep = PI / f32(StackCount);
	f32 SectorAngle, StackAngle;

	for (s32 i = 0; i <= StackCount; i += 1)
	{
		StackAngle = (PI / 2) - (i * StackStep);

		f32 XY = Cos(StackAngle);
		f32 Z = Sin(StackAngle);

		for (s32 j = 0; j <= SectorCount; j += 1)
		{
			SectorAngle = j * SectorStep;

			f32 X = XY * Cos(SectorAngle);
			f32 Y = XY * Sin(SectorAngle);

			// Vertex
			Vertices->Push(X);
			Vertices->Push(Y);
			Vertices->Push(Z);

			// Texcoord
			f32 u = f32(j) / SectorCount;
			f32 v = f32(i) / StackCount;
			Vertices->Push(1 - u);
			Vertices->Push(v);

			// Normal
			/* Vertices->Push(Abs(X)); */
			/* Vertices->Push(Abs(Y)); */
			/* Vertices->Push(Abs(Z)); */
		}
	}

	for (s32 i = 0; i < StackCount; i += 1)
	{
		s32 k1 = i * (SectorCount + 1);
		s32 k2 = k1 + (SectorCount + 1);

		for (s32 j = 0; j < SectorCount; j += 1, k1 += 1, k2 += 1)
		{
			if (i != 0)
			{
				Indices->Push(k1);
				Indices->Push(k2);
				Indices->Push(k1 + 1);
			}
			if (i != (StackCount - 1))
			{
				Indices->Push(k1 + 1);
				Indices->Push(k2);
				Indices->Push(k2 + 1);
			}
		}
	}
}

vertex
SphereVS(renderer_state *State,
		 u32 VertexID)
{
	vertex		Output;
	f32			*Vertices = (f32 *)State->VertexBuffer.Data;
	m4			WVP = State->WVP;
	u32			Stride = 5;

	VertexID *= Stride;
	
	v3 Pos = FetchV3(Vertices, VertexID);
	v2 TexCoord = FetchV2(Vertices, VertexID + 3);

	Output.Pos = WVP * v4(Pos.x, Pos.y, Pos.z, 1.0f);
	Output.TexCoord = TexCoord;

	return (Output);
}

wide_v3
SpherePS(renderer_state *State,
		 vertex_attribs Attribs)
{
	wide_v3		Output;

	Output = SampleTexture(State->Texture, Attribs.TexCoords);

	return (Output);
}

vertex
CubeVS(renderer_state *State,
	   u32 VertexID)
{
	vertex		Output;
	f32			*Vertices = (f32 *)State->VertexBuffer.Data;
	m4			WVP = State->WVP;
	u32			Stride = 6;

	VertexID *= Stride;
	
	v3 Pos = FetchV3(Vertices, VertexID);
	v3 Color = FetchV3(Vertices, VertexID + 3);

	Output.Pos = WVP * v4(Pos.x, Pos.y, Pos.z, 1.0f);
	Output.Color = Color;

	return (Output);
}

wide_v3
CubePS(renderer_state *State,
	   vertex_attribs Attribs)
{
	wide_v3		Output;

	Output = Attribs.Colors;

	return (Output);
}

