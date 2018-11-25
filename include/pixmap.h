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
        static PixelFormat A8R8G8B8;
        static PixelFormat A0R8B8G8;

        int BPP;
        int AlphaShift, RedShift, GreenShift, BlueShift;
        int AlphaBits, RedBits, GreenBits, BlueBits;

        PixelFormat();
        PixelFormat(int bpp, int ashift, int rshift, int gshift, int bshift, int abits, int rbits, int gbits, int bbits);
        size_t PixelsToBytes(size_t pixels);
        bool operator ==(PixelFormat &b);
        bool operator !=(PixelFormat &b);
    };

    union Color
    {
        static Color Black;
        static Color Blue;
        static Color Green;
        static Color Cyan;
        static Color Red;
        static Color Magenta;
        static Color Brown;
        static Color Gray;
        static Color DarkGray;
        static Color BrightBlue;
        static Color BrightGreen;
        static Color BrightCyan;
        static Color BrightRed;
        static Color BrightMagenta;
        static Color Yellow;
        static Color White;
        static Color Transparent;

        struct { uint8_t A, R, G, B; };
        uint32_t Value;

        Color();
        Color(const Color &src);
        Color(uint8_t r, uint8_t g, uint8_t b);
        Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
        Color(uint32_t value);
        static Color FromFloatRGB(float r, float g, float b);
        uint32_t ToValue(PixelFormat &format);
        static Color FromValue(PixelFormat &format, uint32_t value);
        Color Blend(Color c);
    };

    int Width, Height;
    size_t Pitch;
    PixelFormat Format;
    union
    {
        void *Pixels;
        byte *PixelBytes;
    };

    static PixMap *Load(const char *filename);
    static PixMap *LoadCUR(const char *filename, int idx, int *hotX, int *hotY);

    PixMap(int width, int height, PixelFormat format);
    PixMap(int width, int height, size_t pitch, PixelFormat format, void *pixels, bool freePixels);
    PixMap(PixMap *src, PixelFormat format);
    void SetPixel(int x, int y, Color c);
    Color GetPixel(int x, int y);
    void Clear(Color c);
    void HLine(int x1, int y, int x2, Color c);
    void VLine(int x, int y1, int y2, Color c);
    void VFlip();
    void Line(int x1, int y1, int x2, int y2, Color c);
    void Rectangle(int x, int y, int w, int h, Color c);
    void Blit(PixMap *src, int sx, int sy, int x, int y, int w, int h);
    void AlphaBlit(PixMap *src, int sx, int sy, int x, int y, int w, int h);
    void FillRectangle(int x, int y, int w, int h, Color c);
    ~PixMap();
};

#endif // PIXMAP_H
