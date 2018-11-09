#include <pixmap.h>
#include <stdlib.h>
#include <string.h>

PixMap::PixMap(uint width, uint height, PixelFormat format) :
    releasePixels(true),
    Width(width), Height(height),
    Pitch((format.BPP >> 3) * Width),
    Format(format), Pixels(calloc(Height, Pitch))
{
}

PixMap::PixMap(uint width, uint height, size_t pitch, PixMap::PixelFormat format, void *pixels) :
    releasePixels(false),
    Width(width), Height(height), Pitch(pitch),
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

void PixMap::HLine(int x1, int y, int x2, PixMap::Color c)
{
    if(x1 > x2) swap(int, x1, x2);
    if(y < 0 || y >= Height || (x1 < 0 && x2 < 0) || (x1 >= Width && x2 >= Width))
        return;
    x1 = max(0, x1);
    x2 = min(Width - 1, x2);

    uint32_t col = c.ToValue(Format);
    int w = x2 - x1 + 1;

    if(Format.BPP == 32)
    {
        uint32_t *line = (uint32_t *)(PixelBytes + y * Pitch);
        lmemset(line + x1, col, w);
    }
    else if(Format.BPP == 24)
    {
        for(int X = x1; X < x2; ++X)
        {
            uint32_t *pixel = (uint32_t *)(PixelBytes + Pitch * y + X * 3);
            *pixel = (*pixel & 0xFF000000) | col;
        }
    }
    else if(Format.BPP == 16 || Format.BPP == 15)
    {
        uint16_t *line = (uint16_t *)(PixelBytes + y * Pitch);
        wmemset(line + x1, col, w);
    }
}

void PixMap::VLine(int x, int y1, int y2, PixMap::Color c)
{
    // just naive and slow implementation using SetPixel()
    if(y1 > y2) swap(int, y1, y2);
    if(x < 0 || x >= Width || (y1 < 0 && y2 < 0) || (y1 >= Height && y2 >= Height))
        return;
    for(int Y = y1; Y <= y2; ++Y)
        SetPixel(x, Y, c);
}

void PixMap::Line(int x1, int y1, int x2, int y2, PixMap::Color c)
{   // slightly modified version of
    // rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm
    int dx = x2 - x1, sx = dx < 0 ? -1 : 1;
    int dy = y2 - y1, sy = dy < 0 ? -1 : 1;
    dx *= sx; dy *= sy;

    int err = (dx > dy ? dx : -dy) >> 1;

    for(;;)
    {
        SetPixel(x1, y1, c);
        if(x1 == x2 && y1 == y2)
            break;
        int e2 = err;
        if(e2 > -dx)
        {
            err -= dy;
            x1 += sx;
        }
        if(e2 < dy)
        {
            err += dx;
            y1 += sy;
        }
    }
}

void PixMap::Rectangle(int x, int y, int w, int h, PixMap::Color c)
{
    if(w <= 0 || h <= 0) return;
    int x2 = x + w - 1;
    int y2 = y + h - 1;
    HLine(x, y, x2, c);
    HLine(x, y2, x2, c);
    VLine(x, y, y2, c);
    VLine(x2, y, y2, c);
}

void PixMap::FillRectangle(int x, int y, int w, int h, PixMap::Color c)
{
    if(w <= 0 || h <= 0)
        return;

    int x2 = x + w - 1;
    int y2 = y + h;

    if(x2 < 0 || x >= Width || y2 < 0 || y >= Height)
        return;

    for(int Y = y; Y < y2; ++Y)
        HLine(x, Y, x2, c);
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
