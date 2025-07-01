#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <mg.h>

#define BYTES_PER_PIXEL 4

struct bitmap
{
    BITMAPINFO  Info;
    u8          *ColorBuffer;
	u32			*DepthBuffer;
    s32         Width,
                Height;
    HWND        Window;
    HDC         DC;
};

struct color_u8
{
    u8      r,
            g,
            b;
};

void        ClearBitmap(bitmap *Bitmap);
void        AllocateBitmap(bitmap *Bitmap, HWND Window, s32 Width, s32 Height);
void        PresentBitmap(bitmap Bitmap);
void        SetPixel(bitmap *Bitmap, s32 X, s32 Y, color_u8 Color);
u32			PackRGB(u8 R, u8 G, u8 B);

#endif // __BITMAP_H__

