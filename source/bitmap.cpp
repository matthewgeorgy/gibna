#include <bitmap.h>

void		
AllocateBitmap(bitmap *Bitmap, 
			   HWND Window, 
			   s32 Width, 
			   s32 Height)
{
	u32		BitmapMemorySize = Width * Height * BYTES_PER_PIXEL;


	Bitmap->Width = Width;
	Bitmap->Height = Height;
	Bitmap->Window = Window;
	Bitmap->DC = GetDC(Window);
	Bitmap->Memory = (u8 *)HeapAlloc(GetProcessHeap(), 0, BitmapMemorySize);

	Bitmap->Info.bmiHeader.biSize = sizeof(Bitmap->Info.bmiHeader);
	Bitmap->Info.bmiHeader.biWidth = Bitmap->Width;
	Bitmap->Info.bmiHeader.biHeight = Bitmap->Height; // NOTE(matthew): bottom->up bitmap
	Bitmap->Info.bmiHeader.biPlanes = 1;
	Bitmap->Info.bmiHeader.biBitCount = 32;
	Bitmap->Info.bmiHeader.biCompression = BI_RGB;
}

void 		
PresentBitmap(bitmap Bitmap)
{
	StretchDIBits(Bitmap.DC,
		0, 0, Bitmap.Width, Bitmap.Height,
		0, 0, Bitmap.Width, Bitmap.Height,
		Bitmap.Memory, &Bitmap.Info, DIB_RGB_COLORS, SRCCOPY);
}

void 		
SetPixel(bitmap *Bitmap,
		 s32 X, 
		 s32 Y, 
		 color_u8 Color)
{
	s32 PixelCoord = (X + Y * Bitmap->Width) * BYTES_PER_PIXEL;

	Bitmap->Memory[PixelCoord + 0] = Color.b;
	Bitmap->Memory[PixelCoord + 1] = Color.g;
	Bitmap->Memory[PixelCoord + 2] = Color.r;
	Bitmap->Memory[PixelCoord + 3] = 0;
}

void 		
ClearBitmap(bitmap *Bitmap)
{
	ZeroMemory(Bitmap->Memory, Bitmap->Width * Bitmap->Height * BYTES_PER_PIXEL);
}

