#ifndef PIXMAP_H
#define PIXMAP_H

#include <types.h>

// TODO: add indexed pixel format support

class PixMap
{
    bool releasePixels;
public:
    struct PixelFormat
    {
        int BPP;
        int AlphaShift, RedShift, GreenShift, BlueShift;
        int AlphaBits, RedBits, GreenBits, BlueBits;

        PixelFormat();
        PixelFormat(int bpp, int ashift, int rshift, int gshift, int bshift, int abits, int rbits, int gbits, int bbits);
    };

    union Color
    {
        uint32_t Value;
        struct { uint8_t A, R, G, B; };

        Color();
        Color(const Color &src);
        Color(uint8_t r, uint8_t g, uint8_t b);
        Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
        Color(uint32_t value);
        static Color FromFloatRGB(float r, float g, float b);
        uint32_t ToValue(PixelFormat &format);
        static Color FromValue(PixelFormat &format, uint32_t value);
    };

    uint Width, Height;
    size_t Pitch;
    PixelFormat Format;
    union
    {
        void *Pixels;
        byte *PixelBytes;
    };

    PixMap(uint width, uint height, PixelFormat format);
    PixMap(uint width, uint height, size_t pitch, PixelFormat format, void *pixels);
    void SetPixel(int x, int y, Color c);
    Color GetPixel(int x, int y);
    void Clear(Color c);
    void HLine(int x1, int y, int x2, Color c);
    void VLine(int x, int y1, int y2, Color c);
    void Line(int x1, int y1, int x2, int y2, Color c);
    void Rectangle(int x, int y, int w, int h, Color c);
    void FillRectangle(int x, int y, int w, int h, Color c);
    ~PixMap();
};

#endif // PIXMAP_H
