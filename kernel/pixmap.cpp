#include <pixmap.h>
#include <stdlib.h>
#include <string.h>

PixMap::PixMap(uint width, uint heigth, PixelFormat format) :
    releasePixels(true),
    Width(width), Height(heigth),
    Pitch((format.BPP >> 3) * Width),
    Format(format), Pixels(calloc(Height, Pitch))
{
}

PixMap::PixMap(uint width, uint heigth, size_t pitch, PixMap::PixelFormat format, void *pixels) :
    releasePixels(false),
    Width(width), Height(heigth), Pitch(pitch),
    Format(format), Pixels(pixels)
{

}

void PixMap::SetPixel(int x, int y, PixMap::Color c)
{
    if(x < 0 || x >= Width || y < 0 || y >= Height)
        return;
    uint32_t col = c.ToValue(Format);
    if(Format.BPP == 32)
    {
        uint32_t *pixel = (uint32_t *)(PixelBytes + Pitch * y + x * 4);
        *pixel = col;
    }
    else if(Format.BPP == 24)
    {
        uint32_t *pixel = (uint32_t *)(PixelBytes + Pitch * y + x * 3);
        *pixel = (*pixel & 0xFF000000) | col;
    }
    else if(Format.BPP == 16 || Format.BPP == 15)
    {
        uint16_t *pixel = (uint16_t *)(PixelBytes + Pitch * y + x * 2);
        *pixel = col;
    }
}

PixMap::Color PixMap::GetPixel(int x, int y)
{
    if(x < 0 || x >= Width || y < 0 || y >= Height)
        return Color();
    if(Format.BPP == 32)
    {
        uint32_t *pixel = (uint32_t *)(PixelBytes + Pitch * y + x * 4);
        return Color::FromValue(Format, *pixel);
    }
    else if(Format.BPP == 24)
    {
        uint32_t *pixel = (uint32_t *)(PixelBytes + Pitch * y + x * 3);
        return Color::FromValue(Format, *pixel & 0xFFFFFF);
    }
    else if(Format.BPP == 16 || Format.BPP == 15)
    {
        uint16_t *pixel = (uint16_t *)(PixelBytes + Pitch * y + x * 2);
        return Color::FromValue(Format, *pixel);
    }
    return Color();
}

void PixMap::Clear(PixMap::Color c)
{
    FillRectangle(0, 0, Width, Height, c);
}

void PixMap::FillRectangle(int x, int y, int w, int h, PixMap::Color c)
{
    if(w <= 0 || h <= 0)
        return;

    int x2 = x + w;
    int y2 = y + h;

    if(x2 < 0 || x >= Width || y2 < 0 || y >= Height)
        return;

    x = clamp(x, 0, Width);
    y = clamp(y, 0, Height);
    x2 = clamp(x2, 0, Width);
    y2 = clamp(y2, 0, Height);
    w = x2 - x;
    h = y2 - y;

    uint32_t col = c.ToValue(Format);

    if(Format.BPP == 32)
    {
        for(int Y = y; Y < x2; ++Y)
        {
            uint32_t *line = (uint32_t *)(PixelBytes + Y * Pitch);
            lmemset(line + x, col, w);
        }
    }
    else if(Format.BPP == 24)
    {
        for(int Y = y; Y < x2; ++Y)
        {
            for(int X = x; X < x2; ++X)
            {
                uint32_t *pixel = (uint32_t *)(PixelBytes + Pitch * Y + X * 3);
                *pixel = (*pixel & 0xFF000000) | col;
            }
        }
    }
    else if(Format.BPP == 16 || Format.BPP == 15)
    {
        for(int Y = y; Y < x2; ++Y)
        {
            uint16_t *line = (uint16_t *)(PixelBytes + Y * Pitch);
            wmemset(line + x, col, w);
        }
    }
}

PixMap::~PixMap()
{
    if(releasePixels && Pixels) free(Pixels);
}

PixMap::Color::Color()
{
}

PixMap::Color::Color(const PixMap::Color &src) :
    Value(src.Value)
{
}

PixMap::Color::Color(uint8_t r, uint8_t g, uint8_t b) :
    R(r), G(g), B(b)
{
}

PixMap::Color::Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b) :
    A(a), R(r), G(g), B(b)
{
}


PixMap::Color::Color(uint32_t value) :
    Value(value)
{
}

PixMap::Color PixMap::Color::FromFloatRGB(float r, float g, float b)
{
    return Color(clamp((int)(r * 255), 0, 255),
                 clamp((int)(g * 255), 0, 255),
                 clamp((int)(b * 255), 0, 255));
}

uint32_t PixMap::Color::ToValue(PixelFormat &format)
{
    uint32_t a = A >> (8 - format.AlphaBits);
    uint32_t r = R >> (8 - format.RedBits);
    uint32_t g = G >> (8 - format.GreenBits);
    uint32_t b = B >> (8 - format.BlueBits);
    return a << format.AlphaShift | r << format.RedShift | g << format.GreenShift | b << format.BlueShift;
}

PixMap::Color PixMap::Color::FromValue(PixelFormat &format, uint32_t value)
{
    uint32_t A = ((value >> format.AlphaShift) << (8 - format.AlphaBits)) & 0xFF;
    uint32_t R = ((value >> format.RedShift) << (8 - format.RedBits)) & 0xFF;
    uint32_t G = ((value >> format.GreenShift) << (8 - format.GreenBits)) & 0xFF;
    uint32_t B = ((value >> format.BlueShift) << (8 - format.BlueBits)) & 0xFF;
    return Color(A, R, G, B);
}

PixMap::PixelFormat::PixelFormat()
{
    memset(this, 0, sizeof(PixelFormat));
}

PixMap::PixelFormat::PixelFormat(int bpp, int ashift, int rshift, int gshift, int bshift, int abits, int rbits, int gbits, int bbits) :
    BPP(bpp), AlphaShift(ashift), RedShift(rshift), GreenShift(gshift), BlueShift(bshift),
    AlphaBits(abits), RedBits(rbits), GreenBits(gbits), BlueBits(bbits)
{
}
