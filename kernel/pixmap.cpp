#include <file.h>
#include <pixmap.h>
#include <stdlib.h>
#include <string.h>

PixMap::Color PixMap::Color::Black(0x00, 0x00, 0x00);
PixMap::Color PixMap::Color::Blue(0x00, 0x00, 0xAA);
PixMap::Color PixMap::Color::Green(0x00, 0xAA, 0x00);
PixMap::Color PixMap::Color::Cyan(0x00, 0xAA, 0xAA);
PixMap::Color PixMap::Color::Red(0xAA, 0x00, 0x00);
PixMap::Color PixMap::Color::Magenta(0xAA, 0x00, 0xAA);
PixMap::Color PixMap::Color::Brown(0xAA, 0x55, 0x00);
PixMap::Color PixMap::Color::Gray(0xAA, 0xAA, 0xAA);
PixMap::Color PixMap::Color::DarkGray(0x55, 0x55, 0x55);
PixMap::Color PixMap::Color::BrightBlue(0x55, 0x55, 0xFF);
PixMap::Color PixMap::Color::BrightGreen(0x55, 0xFF, 0x55);
PixMap::Color PixMap::Color::BrightCyan(0x55, 0xFF, 0xFF);
PixMap::Color PixMap::Color::BrightRed(0xFF, 0x55, 0x55);
PixMap::Color PixMap::Color::BrightMagenta(0xFF, 0x55, 0xFF);
PixMap::Color PixMap::Color::Yellow(0xFF, 0xFF, 0x55);
PixMap::Color PixMap::Color::White(0xFF, 0xFF, 0xFF);

#pragma pack(push, 1)
struct BMPImageHeader
{
    uint32_t Size;
    uint32_t Width;
    int32_t Height;
    uint16_t Planes;
    uint16_t BitCount;
    uint32_t Compression;
    uint32_t SizeImage;
    uint32_t XPelsPerMeter;
    uint32_t YPelsPerMeter;
    uint32_t ClrUsed;
    uint32_t ClrImportant;
};

struct BMPFileHeader
{
    uint16_t Type;
    uint32_t Size;
    uint16_t Reserved[2];
    uint32_t OffBits;
    BMPImageHeader Image;
};
#pragma pack(pop)

#define BMP_MAGIC 0x4D42

PixMap *PixMap::Load(const char *filename)
{
    File *f = File::Open(filename, O_RDONLY);
    if(!f) return nullptr;
    BMPFileHeader hdr;
    if(f->Read(&hdr, sizeof(hdr)) != sizeof(hdr))
    {
        delete f;
        return nullptr;
    }
    if(hdr.Type != BMP_MAGIC || hdr.Image.Size < 40 || hdr.Image.BitCount < 16)
    {
        delete f;
        return nullptr;
    }
    if(f->Seek(hdr.OffBits, SEEK_SET) != hdr.OffBits)
    {
        delete f;
        return nullptr;
    }
    size_t pitch = align(hdr.Image.Width * hdr.Image.BitCount, 8) / 8;
    int height = hdr.Image.Height < 0 ? -hdr.Image.Height : hdr.Image.Height;
    void *pixels = calloc(height, pitch);
    if(!pixels)
    {
        delete f;
        return nullptr;
    }
    size_t size = height * pitch;
    if(f->Read(pixels, size) != size)
    {
        free(pixels);
        delete f;
        return nullptr;
    }
    delete f;
    int abits = hdr.Image.BitCount == 32 ? 8 : 0;
    int ashift = hdr.Image.BitCount == 32 ? 24 : 0;
    int rbits = hdr.Image.BitCount >= 24 ? 8 : 5;
    int rshift = hdr.Image.BitCount >= 24 ? 16 : 11;
    int gbits = hdr.Image.BitCount >= 24 ? 8 : 6;
    int gshift = hdr.Image.BitCount >= 24 ? 8 : 5;
    int bbits = hdr.Image.BitCount >= 24 ? 8 : 5;
    int bshift = 0;
    PixelFormat format(hdr.Image.BitCount, ashift, rshift, gshift, bshift, abits, rbits, gbits, bbits);
    PixMap *res = new PixMap(hdr.Image.Width, height, pitch, format, pixels, true);
    if(hdr.Image.Height > 0)
        res->VFlip();
    return res;
}

PixMap::PixMap(int width, int height, PixelFormat format) :
    releasePixels(true),
    Width(width), Height(height),
    Pitch((format.BPP >> 3) * Width),
    Format(format), Pixels(calloc(Height, Pitch))
{
}

PixMap::PixMap(int width, int height, size_t pitch, PixMap::PixelFormat format, void *pixels, bool freePixels) :
    releasePixels(freePixels),
    Width(width), Height(height), Pitch(pitch),
    Format(format), Pixels(pixels)
{
}

PixMap::PixMap(PixMap *src, PixMap::PixelFormat format) :
    PixMap(src->Width, src->Height, format)
{
    Blit(src, 0, 0, 0, 0, Width, Height);
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
    x = x < 0 ? Width - (-x % Width) : x % Width;
    y = y < 0 ? Height - (-y % Height) : y % Height;

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

void PixMap::VFlip()
{
    void *lineBuf = malloc(Pitch);
    if(!lineBuf) return;
    for(int y = 0; y < Height / 2; ++y)
    {
        memcpy(lineBuf, PixelBytes + Pitch * (Height - (y + 1)), Pitch);
        memcpy(PixelBytes + Pitch * (Height - (y + 1)), PixelBytes + Pitch * y, Pitch);
        memcpy(PixelBytes + Pitch * y, lineBuf, Pitch);
    }
    free(lineBuf);
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

void PixMap::Blit(PixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    if(x < 0)
    {
        sx -= x;
        w += x;
        x = 0;
    }
    if(y < 0)
    {
        sy -= y;
        h += y;
        y = 0;
    }
    if(sx < 0)
    {
        x -= sx;
        w += sx;
        sx = 0;
    }
    if(sy < 0)
    {
        y -= sy;
        h += sy;
        sy = 0;
    }
    if(w < 0 || h < 0 || x >= Width || y >= Height)
        return;

    int x2 = min(Width, x + w);
    int y2 = min(Height, y + h);
    if(x2 <= 0 || y2 <= 0 || sx >= src->Width || sy >= src->Height)
        return;

    int sx2 = min(src->Width, sx + w);
    int sy2 = min(src->Height, sy + h);

    byte *d = PixelBytes + y * Pitch + Format.PixelsToBytes(x);
    byte *s = src->PixelBytes + sy * src->Pitch + Format.PixelsToBytes(sx);
    if(Format == src->Format && Format.BPP >= 8)
    {   // use fast blit if pixel formats match
        int sw = sx2 - sx;
        int sh = sy2 - sy;
        int W = x2 - x;
        int H = y2 - y;
        if(W < 0 || H < 0)
            return;
        size_t bpl = Format.PixelsToBytes(min(sw, W));
        bltmove(d, s, bpl, Pitch, src->Pitch, min(sh, H));
        return;
    }

    // fall back to slow GetPixel/SetPixel implementation
    if(s == d)
        return; // nothing to do
    else if(s > d)
    {   // forward
        for(int Y = y, sY = sy; Y < y2 && sY < sy2; ++Y, ++sY)
        {
            for(int X = x, sX = sx; X < x2 && sX < sx2; ++X, ++sX)
                SetPixel(X, Y, src->GetPixel(sX, sY));
        }
    }
    else
    {   // backward
        for(int Y = y2 - 1, sY = sy2 - 1; Y >= y && sY >= sy; --Y, --sY)
        {
            for(int X = x2 - 1, sX = sx2 - 1; X >= x && sX >= sx; --X, --sX)
                SetPixel(X, Y, src->GetPixel(sX, sY));
        }
    }
}

void PixMap::AlphaBlit(PixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    if(!src->Format.AlphaBits)
        return Blit(src, sx, sy, x, y, w, h);

    if(x < 0)
    {
        sx -= x;
        w += x;
        x = 0;
    }
    if(y < 0)
    {
        sy -= y;
        h += y;
        y = 0;
    }
    if(sx < 0)
    {
        x -= sx;
        w += sx;
        sx = 0;
    }
    if(sy < 0)
    {
        y -= sy;
        h += sy;
        sy = 0;
    }
    if(w < 0 || h < 0 || x >= Width || y >= Height)
        return;

    int x2 = min(Width, x + w);
    int y2 = min(Height, y + h);
    if(x2 <= 0 || y2 <= 0 || sx >= src->Width || sy >= src->Height)
        return;

    int sx2 = min(src->Width, sx + w);
    int sy2 = min(src->Height, sy + h);

    byte *d = PixelBytes + y * Pitch + Format.PixelsToBytes(x);
    byte *s = src->PixelBytes + sy * src->Pitch + Format.PixelsToBytes(sx);

    // slow !!!
    if(s == d)
        return; // nothing to do
    else if(s > d)
    {   // forward
        for(int Y = y, sY = sy; Y < y2 && sY < sy2; ++Y, ++sY)
        {
            for(int X = x, sX = sx; X < x2 && sX < sx2; ++X, ++sX)
                SetPixel(X, Y, GetPixel(X, Y).Blend(src->GetPixel(sX, sY)));
        }
    }
    else
    {   // backward
        for(int Y = y2 - 1, sY = sy2 - 1; Y >= y && sY >= sy; --Y, --sY)
        {
            for(int X = x2 - 1, sX = sx2 - 1; X >= x && sX >= sx; --X, --sX)
                SetPixel(X, Y, GetPixel(X, Y).Blend(src->GetPixel(sX, sY)));
        }
    }
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

PixMap::Color PixMap::Color::Blend(PixMap::Color c)
{
    unsigned int A = c.A + 1;
    unsigned int iA = 256 - c.A;
    return Color(this->A, (A * c.R + iA * R) >> 8, (A * c.G + iA * G) >> 8, (A * c.B + iA * B) >> 8);
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

size_t PixMap::PixelFormat::PixelsToBytes(size_t pixels)
{
    return align(pixels * BPP, 8) / 8;
}

bool PixMap::PixelFormat::operator ==(PixMap::PixelFormat &b)
{
    return BPP == b.BPP && AlphaShift == b.AlphaShift && RedShift == b.RedShift && GreenShift == b.GreenShift && BlueShift == b.BlueShift &&
            AlphaBits == b.AlphaBits && RedBits == b.RedBits && GreenBits == b.GreenBits && BlueBits == b.BlueBits;
}

bool PixMap::PixelFormat::operator !=(PixMap::PixelFormat &b)
{
    return BPP != b.BPP || AlphaShift != b.AlphaShift || RedShift != b.RedShift || GreenShift != b.GreenShift || BlueShift != b.BlueShift ||
            AlphaBits != b.AlphaBits || RedBits != b.RedBits || GreenBits != b.GreenBits || BlueBits != b.BlueBits;
}
