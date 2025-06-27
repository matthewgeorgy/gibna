#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <mg.h>

#define BYTES_PER_PIXEL	4

struct bitmap
{
	BITMAPINFO	Info;
	u8			*Memory;
	s32 		Width,
				Height;
	HWND 		Window;
	HDC			DC;
};

void		AllocateBitmap(bitmap *Bitmap, HWND Window, s32 Width, s32 Height);
void 		PresentBitmap(bitmap Bitmap);

#endif // __BITMAP_H__

