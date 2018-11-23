#include <stdlib.h>
#include <string.h>
#include <woot/pixmap.h>

#define swap(type, a, b) { type tmp = a; a = b; b = tmp; }
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

extern void lmemset(void *ptr, unsigned int value, unsigned long num);

static void *bltmove(void *dst, const void *src, size_t bpl, size_t dstride, size_t sstride, size_t lines)
{
    unsigned char *d = (unsigned char *)dst;
    unsigned char *s = (unsigned char *)src;
    int fwd = d < s;

    if(fwd)
    {
        while(lines--)
        {
            memmove(d, s, bpl);
            d += dstride;
            s += sstride;
        }
    }
    else
    {
        d += dstride * lines;
        s += sstride * lines;
        while(lines--)
        {
            d -= dstride;
            s -= sstride;
            memmove(d, s, bpl);
        }
    }
    return dst;
}

int pmFormatEqual(struct pmPixelFormat a, struct pmPixelFormat b)
{
    return a.BPP == b.BPP && a.AlphaShift == b.AlphaShift && a.RedShift == b.RedShift && a.GreenShift == b.GreenShift && a.BlueShift == b.BlueShift &&
            a.AlphaBits == b.AlphaBits && a.RedBits == b.RedBits && a.GreenBits == b.GreenBits && a.BlueBits == b.BlueBits;
}

unsigned int pmFormatPixelsToBytes(struct pmPixelFormat format, unsigned int pixels)
{
    return (pixels * format.BPP + 7) / 8;
}

union pmColor pmColorFromRGB(unsigned char r, unsigned char g, unsigned char b)
{
    union pmColor color;
    color.A = 0;
    color.R = r;
    color.G = g;
    color.B = b;
    return color;
}

union pmColor pmColorFromARGB(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
    union pmColor color;
    color.A = a;
    color.R = r;
    color.G = g;
    color.B = b;
    return color;
}

unsigned int pmColorToValue(union pmColor color, struct pmPixelFormat format)
{
    unsigned int a = color.A >> (8 - format.AlphaBits);
    unsigned int r = color.R >> (8 - format.RedBits);
    unsigned int g = color.G >> (8 - format.GreenBits);
    unsigned int b = color.B >> (8 - format.BlueBits);
    return a << format.AlphaShift | r << format.RedShift | g << format.GreenShift | b << format.BlueShift;
}

union pmColor pmColorFromValue(struct pmPixelFormat format, unsigned int value)
{
    unsigned int A = ((value >> format.AlphaShift) << (8 - format.AlphaBits)) & 0xFF;
    unsigned int R = ((value >> format.RedShift) << (8 - format.RedBits)) & 0xFF;
    unsigned int G = ((value >> format.GreenShift) << (8 - format.GreenBits)) & 0xFF;
    unsigned int B = ((value >> format.BlueShift) << (8 - format.BlueBits)) & 0xFF;
    return pmColorFromARGB(A, R, G, B);
}

struct pmPixMap *pmCreate(int width, int height, struct pmPixelFormat format)
{
    if(width < 1 || height < 1 || (format.BPP != 15 && format.BPP != 16 && format.BPP != 24 && format.BPP != 32))
        return NULL;
    struct pmPixMap *pixMap = (struct pmPixMap *)calloc(1, sizeof(struct pmPixMap));
    if(!pixMap) return NULL;
    pixMap->Width = width;
    pixMap->Height = height;
    pixMap->Pitch = width * (format.BPP + 7) / 8;
    pixMap->Format = format;
    pixMap->Pixels = calloc(height, pixMap->Pitch);
    if(!pixMap->Pixels)
    {
        pmDelete(pixMap);
        return NULL;
    }
    return pixMap;
}

struct pmPixMap *pmCreate2(int width, int height, int pitch, struct pmPixelFormat format, void *pixels, int releasePixels)
{
    if(width < 1 || height < 1 || (format.BPP != 15 && format.BPP != 16 && format.BPP != 24 && format.BPP != 32))
        return NULL;
    struct pmPixMap *pixMap = (struct pmPixMap *)calloc(1, sizeof(struct pmPixMap));
    if(!pixMap) return NULL;
    pixMap->Width = width;
    pixMap->Height = height;
    pixMap->Pitch = pitch;
    pixMap->Format = format;
    pixMap->Pixels = pixels;
    return pixMap;
}

struct pmPixMap *pmFromPixMap(struct pmPixMap *src, struct pmPixelFormat format)
{
    struct pmPixMap *pixMap = pmCreate(src->Width, src->Height, format);
    if(!pixMap) return NULL;
    pmBlit(pixMap, src, 0, 0, 0, 0, pixMap->Width, pixMap->Height);
    return pixMap;
}

void pmSetPixel(struct pmPixMap *pixMap, int x, int y, union pmColor color)
{
    if(x < 0 || x >= pixMap->Width || y < 0 || y >= pixMap->Height)
        return;
    unsigned int col = pmColorToValue(color, pixMap->Format);
    if(pixMap->Format.BPP == 32)
    {
        unsigned int *pixel = (unsigned int *)(pixMap->PixelBytes + pixMap->Pitch * y + x * 4);
        *pixel = col;
    }
    else if(pixMap->Format.BPP == 24)
    {
        unsigned int *pixel = (unsigned int *)(pixMap->PixelBytes + pixMap->Pitch * y + x * 3);
        *pixel = (*pixel & 0xFF000000) | col;
    }
    else if(pixMap->Format.BPP == 16 || pixMap->Format.BPP == 15)
    {
        unsigned short *pixel = (unsigned short *)(pixMap->PixelBytes + pixMap->Pitch * y + x * 2);
        *pixel = col;
    }
}

union pmColor pmGetPixel(struct pmPixMap *pixMap, int x, int y)
{
    x = x < 0 ? pixMap->Width - (-x % pixMap->Width) : x % pixMap->Width;
    y = y < 0 ? pixMap->Height - (-y % pixMap->Height) : y % pixMap->Height;

    if(pixMap->Format.BPP == 32)
    {
        unsigned int *pixel = (unsigned int *)(pixMap->PixelBytes + pixMap->Pitch * y + x * 4);
        return pmColorFromValue(pixMap->Format, *pixel);
    }
    else if(pixMap->Format.BPP == 24)
    {
        unsigned int *pixel = (unsigned int *)(pixMap->PixelBytes + pixMap->Pitch * y + x * 3);
        return pmColorFromValue(pixMap->Format, *pixel & 0xFFFFFF);
    }
    else if(pixMap->Format.BPP == 16 || pixMap->Format.BPP == 15)
    {
        unsigned short *pixel = (unsigned short *)(pixMap->PixelBytes + pixMap->Pitch * y + x * 2);
        return pmColorFromValue(pixMap->Format, *pixel);
    }
    return pmColorFromValue(pixMap->Format, 0);
}

union pmColor pmBlendPixel(union pmColor a, union pmColor b)
{
    unsigned int A = b.A + 1;
    unsigned int iA = 256 - b.A;
    return pmColorFromARGB(a.A, (A * b.R + iA * a.R) >> 8, (A * b.G + iA * a.G) >> 8, (A * b.B + iA * a.B) >> 8);
}

void pmHLine(struct pmPixMap *pixMap, int x1, int y, int x2, union pmColor c)
{
    if(x1 > x2) swap(int, x1, x2);
    if(y < 0 || y >= pixMap->Height || (x1 < 0 && x2 < 0) || (x1 >= pixMap->Width && x2 >= pixMap->Width))
        return;
    x1 = max(0, x1);
    x2 = min(pixMap->Width - 1, x2);

    unsigned int col = pmColorToValue(c, pixMap->Format);
    int w = x2 - x1 + 1;

    if(pixMap->Format.BPP == 32)
    {
        unsigned int *line = (unsigned int *)(pixMap->PixelBytes + y * pixMap->Pitch);
        lmemset(line + x1, col, w);
    }
    else if(pixMap->Format.BPP == 24)
    {
        for(int X = x1; X < x2; ++X)
        {
            unsigned int *pixel = (unsigned int *)(pixMap->PixelBytes + pixMap->Pitch * y + X * 3);
            *pixel = (*pixel & 0xFF000000) | col;
        }
    }
    else if(pixMap->Format.BPP == 16 || pixMap->Format.BPP == 15)
    {
        unsigned short *line = (unsigned short *)(pixMap->PixelBytes + y * pixMap->Pitch);
        wmemset(line + x1, col, w);
    }
}

void pmVLine(struct pmPixMap *pixMap, int x, int y1, int y2, union pmColor c)
{
    // just naive and slow implementation using SetPixel()
    if(y1 > y2) swap(int, y1, y2);
    if(x < 0 || x >= pixMap->Width || (y1 < 0 && y2 < 0) || (y1 >= pixMap->Height && y2 >= pixMap->Height))
        return;
    for(int Y = y1; Y <= y2; ++Y)
        pmSetPixel(pixMap, x, Y, c);
}

void pmVFlip(struct pmPixMap *pixMap)
{
    void *lineBuf = malloc(pixMap->Pitch);
    if(!lineBuf) return;
    for(int y = 0; y < pixMap->Height / 2; ++y)
    {
        memcpy(lineBuf, pixMap->PixelBytes + pixMap->Pitch * (pixMap->Height - (y + 1)), pixMap->Pitch);
        memcpy(pixMap->PixelBytes + pixMap->Pitch * (pixMap->Height - (y + 1)), pixMap->PixelBytes + pixMap->Pitch * y, pixMap->Pitch);
        memcpy(pixMap->PixelBytes + pixMap->Pitch * y, lineBuf, pixMap->Pitch);
    }
    free(lineBuf);
}

void pmLine(struct pmPixMap *pixMap, int x1, int y1, int x2, int y2, union pmColor c)
{
    int dx = x2 - x1, sx = dx < 0 ? -1 : 1;
    int dy = y2 - y1, sy = dy < 0 ? -1 : 1;
    dx *= sx; dy *= sy;

    int err = (dx > dy ? dx : -dy) >> 1;

    for(;;)
    {
        pmSetPixel(pixMap, x1, y1, c);
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

void pmRectangle(struct pmPixMap *pixMap, int x, int y, int w, int h, union pmColor c)
{
    if(w <= 0 || h <= 0) return;
    int x2 = x + w - 1;
    int y2 = y + h - 1;
    pmHLine(pixMap, x, y, x2, c);
    pmHLine(pixMap, x, y2, x2, c);
    pmVLine(pixMap, x, y, y2, c);
    pmVLine(pixMap, x2, y, y2, c);
}

void pmFillRectangle(struct pmPixMap *pixMap, int x, int y, int w, int h, union pmColor c)
{
    if(w <= 0 || h <= 0)
        return;

    int x2 = x + w - 1;
    int y2 = y + h;

    if(x2 < 0 || x >= pixMap->Width || y2 < 0 || y >= pixMap->Height)
        return;

    for(int Y = y; Y < y2; ++Y)
        pmHLine(pixMap, x, Y, x2, c);
}

void pmBlit(struct pmPixMap *dst, struct pmPixMap *src, int sx, int sy, int x, int y, int w, int h)
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
    if(w < 0 || h < 0 || x >= dst->Width || y >= dst->Height)
        return;

    int x2 = min(dst->Width, x + w);
    int y2 = min(dst->Height, y + h);
    if(x2 <= 0 || y2 <= 0 || sx >= src->Width || sy >= src->Height)
        return;

    int sx2 = min(src->Width, sx + w);
    int sy2 = min(src->Height, sy + h);

    unsigned char *d = dst->PixelBytes + y * dst->Pitch + pmFormatPixelsToBytes(dst->Format, x);
    unsigned char *s = src->PixelBytes + sy * src->Pitch + pmFormatPixelsToBytes(src->Format, sx);
    if(pmFormatEqual(dst->Format, src->Format) && dst->Format.BPP >= 8)
    {   // use fast blit if pixel formats match
        int sw = sx2 - sx;
        int sh = sy2 - sy;
        int W = x2 - x;
        int H = y2 - y;
        if(W < 0 || H < 0)
            return;
        size_t bpl = pmFormatPixelsToBytes(dst->Format, min(sw, W));
        bltmove(d, s, bpl, dst->Pitch, src->Pitch, min(sh, H));
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
                pmSetPixel(dst, X, Y, pmGetPixel(src, sX, sY));
        }
    }
    else
    {   // backward
        for(int Y = y2 - 1, sY = sy2 - 1; Y >= y && sY >= sy; --Y, --sY)
        {
            for(int X = x2 - 1, sX = sx2 - 1; X >= x && sX >= sx; --X, --sX)
                pmSetPixel(dst, X, Y, pmGetPixel(src, sX, sY));
        }
    }
}

void pmDelete(struct pmPixMap *pixMap)
{
    if(!pixMap) return;
    if(pixMap->ReleasePixels && pixMap->Pixels)
        free(pixMap->Pixels);
    free(pixMap);
}

void pmAlphaBlit(struct pmPixMap *dst, struct pmPixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    if(!src->Format.AlphaBits)
        return pmBlit(dst, src, sx, sy, x, y, w, h);

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
    if(w < 0 || h < 0 || x >= dst->Width || y >= dst->Height)
        return;

    int x2 = min(dst->Width, x + w);
    int y2 = min(dst->Height, y + h);
    if(x2 <= 0 || y2 <= 0 || sx >= src->Width || sy >= src->Height)
        return;

    int sx2 = min(src->Width, sx + w);
    int sy2 = min(src->Height, sy + h);

    unsigned char *d = dst->PixelBytes + y * dst->Pitch + pmFormatPixelsToBytes(dst->Format, x);
    unsigned char *s = src->PixelBytes + sy * src->Pitch + pmFormatPixelsToBytes(src->Format, sx);

    // slow !!!
    if(s == d)
        return; // nothing to do
    else if(s > d)
    {   // forward
        for(int Y = y, sY = sy; Y < y2 && sY < sy2; ++Y, ++sY)
        {
            for(int X = x, sX = sx; X < x2 && sX < sx2; ++X, ++sX)
                pmSetPixel(dst, X, Y, pmBlendPixel(pmGetPixel(dst, X, Y), pmGetPixel(src, sX, sY)));
        }
    }
    else
    {   // backward
        for(int Y = y2 - 1, sY = sy2 - 1; Y >= y && sY >= sy; --Y, --sY)
        {
            for(int X = x2 - 1, sX = sx2 - 1; X >= x && sX >= sx; --X, --sX)
                pmSetPixel(dst, X, Y, pmBlendPixel(pmGetPixel(dst, X, Y), pmGetPixel(src, sX, sY)));
        }
    }
}
