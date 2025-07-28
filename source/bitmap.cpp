#include <bitmap.h>

#define MAX_SCR_WIDTH	2048
#define MAX_SCR_HEIGHT	2048

void
AllocateBitmap(bitmap *Bitmap,
			   HWND Window,
			   s32 Width,
			   s32 Height)
{
	u32		BitmapMemorySize = MAX_SCR_WIDTH * MAX_SCR_HEIGHT * BYTES_PER_PIXEL;

	Bitmap->Width = Width;
	Bitmap->Height = Height;
	Bitmap->Window = Window;
	Bitmap->DC = GetDC(Window);
	Bitmap->ColorBuffer = (u32 *)VirtualAlloc(NULL, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	Bitmap->DepthBuffer = (u32 *)VirtualAlloc(NULL, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	Bitmap->Info.bmiHeader.biSize = sizeof(Bitmap->Info.bmiHeader);
	Bitmap->Info.bmiHeader.biWidth = Bitmap->Width;
	Bitmap->Info.bmiHeader.biHeight = Bitmap->Height; // NOTE(matthew): bottom->up bitmap
	Bitmap->Info.bmiHeader.biPlanes = 1;
	Bitmap->Info.bmiHeader.biBitCount = 32;
	Bitmap->Info.bmiHeader.biCompression = BI_RGB;
}

void		
ResizeBitmap(bitmap *Bitmap, 
			 s32 NewWidth, 
			 s32 NewHeight)
{
	Bitmap->Width = NewWidth;
	Bitmap->Height = NewHeight;
	Bitmap->Info.bmiHeader.biWidth = NewWidth;
	Bitmap->Info.bmiHeader.biHeight = NewHeight; // NOTE(matthew): bottom->up bitmap
}

void
PresentBitmap(bitmap Bitmap)
{
	StretchDIBits(Bitmap.DC,
		0, 0, Bitmap.Width, Bitmap.Height,
		0, 0, Bitmap.Width, Bitmap.Height,
		Bitmap.ColorBuffer, &Bitmap.Info, DIB_RGB_COLORS, SRCCOPY);
}

void
SetPixel(bitmap *Bitmap,
		 s32 X,
		 s32 Y,
		 color_u8 Color)
{
	s32 PixelCoord = X + Y * Bitmap->Width;
	u32 RGB = PackRGB(Color.r, Color.g, Color.b);

	*(u32 *)(&Bitmap->ColorBuffer[PixelCoord]) = RGB;
}

void
ClearBitmap(bitmap *Bitmap)
{
	ZeroMemory(Bitmap->ColorBuffer, Bitmap->Width * Bitmap->Height * BYTES_PER_PIXEL);
	for (s32 I = 0; I < Bitmap->Width * Bitmap->Height; I += 1)
	{
		Bitmap->DepthBuffer[I] = 0x7FFFFFFF;
	}
}

u32
PackRGB(u8 R,
		u8 G,
		u8 B)
{
	u32		RGB = (R << 16) | (G << 8) | (B << 0);

	return (RGB);
}

