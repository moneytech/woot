#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <woot/pixmap.h>
#include <woot/wm.h>

#define swap(type, a, b) { type tmp = a; a = b; b = tmp; }
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

struct pmPixMap
{
    struct pmPixMap *Parent;
    struct wmRectangle Contents;
    struct wmRectangle Dirty;
    int Pitch;
    struct pmPixelFormat Format;
    int ReleasePixels;
    union
    {
        void *Pixels;
        unsigned char *PixelBytes;
    };
};

union pmColor pmColorBlack = { 0xFF, 0x00, 0x00, 0x00 };
union pmColor pmColorBlue = { 0xFF, 0x00, 0x00, 0xAA };
union pmColor pmColorGreen = { 0xFF, 0x00, 0xAA, 0x00 };
union pmColor pmColorCyan = { 0xFF, 0x00, 0xAA, 0xAA };
union pmColor pmColorRed = { 0xFF, 0xAA, 0x00, 0x00 };
union pmColor pmColorMagenta = { 0xFF, 0xAA, 0x00, 0xAA };
union pmColor pmColorBrown = { 0xFF, 0xAA, 0x55, 0x00 };
union pmColor pmColorGray = { 0xFF, 0xAA, 0xAA, 0xAA };
union pmColor pmColorDarkGray = { 0xFF, 0x55, 0x55, 0x55 };
union pmColor pmColorBrightBlue = { 0xFF, 0x55, 0x55, 0xFF };
union pmColor pmColorBrightGreen = { 0xFF, 0x55, 0xFF, 0x55 };
union pmColor pmColorBrightCyan = { 0xFF, 0x55, 0xFF, 0xFF };
union pmColor pmColorBrightRed = { 0xFF, 0xFF, 0x55, 0x55 };
union pmColor pmColorBrightMagenta = { 0xFF, 0xFF, 0x55, 0xFF };
union pmColor pmColorYellow = { 0xFF, 0xFF, 0xFF, 0x55 };
union pmColor pmColorWhite = { 0xFF, 0xFF, 0xFF, 0xFF };
union pmColor pmColorTransparent = { 0x00, 0x00, 0x00, 0x00 };

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
    pixMap->Contents.Width = width;
    pixMap->Contents.Height = height;
    pixMap->Pitch = width * (format.BPP + 7) / 8;
    pixMap->Format = format;
    pixMap->Pixels = calloc(height, pixMap->Pitch);
    pixMap->ReleasePixels = 1;
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
    pixMap->Contents.Width = width;
    pixMap->Contents.Height = height;
    pixMap->Pitch = pitch;
    pixMap->Format = format;
    pixMap->Pixels = pixels;
    pixMap->ReleasePixels = releasePixels;
    return pixMap;
}

struct pmPixMap *pmFromPixMap(struct pmPixMap *src, struct pmPixelFormat format)
{
    struct pmPixMap *pixMap = pmCreate(src->Contents.Width, src->Contents.Height, format);
    if(!pixMap) return NULL;
    pmBlit(pixMap, src, 0, 0, 0, 0, pixMap->Contents.Width, pixMap->Contents.Height);
    return pixMap;
}

struct pmPixMap *pmSubPixMap(struct pmPixMap *src, int x, int y, int w, int h)
{
    struct wmRectangle srcRect = { 0, 0, src->Contents.Width, src->Contents.Height };
    struct wmRectangle newRect = { x, y, w, h };
    newRect = wmRectangleIntersection(srcRect, newRect);
    if(newRect.Width <= 0 || newRect.Height <= 0)
        return NULL;
    struct pmPixMap *pm = pmCreate2(newRect.Width, newRect.Height, src->Pitch, src->Format, src->PixelBytes + newRect.Y * src->Pitch + pmFormatPixelsToBytes(src->Format, newRect.X), 0);
    pm->Parent = src;
    pm->Contents.X = x;
    pm->Contents.Y = y;
    return pm;
}

struct pmPixMap *pmLoadPNG(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if(!f) return NULL;
    char header[8];
    if(fread(header, sizeof(header), 1, f) != 1 || png_sig_cmp(header, 0, sizeof(header)))
    {
        fclose(f);
        return NULL;
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
    {
        fclose(f);
        return NULL;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(f);
        return NULL;
    }
    png_init_io(png_ptr, f);
    png_set_sig_bytes(png_ptr, sizeof(header));
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_read_update_info(png_ptr, info_ptr);

    // only 24 and 32 bit pngs are supported for now
    if(bit_depth != 8 || (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(f);
        return NULL;
    }

    png_bytep *row_pointers = (png_bytep *)calloc(height, sizeof(png_bytep));
    if(!row_pointers)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(f);
        return NULL;
    }

    size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    void *pixels = calloc(height, row_bytes);
    if(!pixels)
    {
        free(row_pointers);
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(f);
        return NULL;
    }

    int a = color_type == PNG_COLOR_TYPE_RGBA;
    struct pmPixelFormat pf = { 32, a ? 24 : 0, 0, 8, 16, a ? 8 : 0, 8, 8, 8 };
    struct pmPixMap *pm = pmCreate2(width, height, row_bytes, pf, pixels, 1);
    if(!pm)
    {
        free(pixels);
        free(row_pointers);
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(f);
        return NULL;
    }

    for(png_uint_32 y = 0; y < height; ++y)
        row_pointers[y] = pm->PixelBytes + pm->Pitch * y;

    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(f);

    free(row_pointers);

    return pm;
}

struct wmRectangle pmGetRectangle(struct pmPixMap *pixMap)
{
    if(!pixMap) return wmRectangleEmpty;
    return pixMap->Contents;
}

int pmGetPitch(struct pmPixMap *pixMap)
{
    if(!pixMap) return -EINVAL;
    return pixMap->Pitch;
}

void *pmGetPixels(struct pmPixMap *pixMap)
{
    if(!pixMap) return NULL;
    return pixMap->Pixels;
}

void pmSetPixel(struct pmPixMap *pixMap, int x, int y, union pmColor color)
{
    if(x < 0 || x >= pixMap->Contents.Width || y < 0 || y >= pixMap->Contents.Height)
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
    x = x < 0 ? pixMap->Contents.Width - (-x % pixMap->Contents.Width) : x % pixMap->Contents.Width;
    y = y < 0 ? pixMap->Contents.Height - (-y % pixMap->Contents.Height) : y % pixMap->Contents.Height;

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
    if(y < 0 || y >= pixMap->Contents.Height || (x1 < 0 && x2 < 0) || (x1 >= pixMap->Contents.Width && x2 >= pixMap->Contents.Width))
        return;
    x1 = max(0, x1);
    x2 = min(pixMap->Contents.Width - 1, x2);

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
    if(x < 0 || x >= pixMap->Contents.Width || (y1 < 0 && y2 < 0) || (y1 >= pixMap->Contents.Height && y2 >= pixMap->Contents.Height))
        return;
    for(int Y = y1; Y <= y2; ++Y)
        pmSetPixel(pixMap, x, Y, c);
}

void pmVFlip(struct pmPixMap *pixMap)
{
    void *lineBuf = malloc(pixMap->Pitch);
    if(!lineBuf) return;
    for(int y = 0; y < pixMap->Contents.Height / 2; ++y)
    {
        memcpy(lineBuf, pixMap->PixelBytes + pixMap->Pitch * (pixMap->Contents.Height - (y + 1)), pixMap->Pitch);
        memcpy(pixMap->PixelBytes + pixMap->Pitch * (pixMap->Contents.Height - (y + 1)), pixMap->PixelBytes + pixMap->Pitch * y, pixMap->Pitch);
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

void pmRectangleRect(struct pmPixMap *pixMap, struct wmRectangle rect, union pmColor c)
{
    pmRectangle(pixMap, rect.X, rect.Y, rect.Width, rect.Height, c);
}

void pmFillRectangle(struct pmPixMap *pixMap, int x, int y, int w, int h, union pmColor c)
{
    if(w <= 0 || h <= 0)
        return;

    int x2 = x + w - 1;
    int y2 = y + h;

    if(x2 < 0 || x >= pixMap->Contents.Width || y2 < 0 || y >= pixMap->Contents.Height)
        return;

    for(int Y = y; Y < y2; ++Y)
        pmHLine(pixMap, x, Y, x2, c);
}

void pmClear(struct pmPixMap *pixMap, union pmColor color)
{
    pmFillRectangle(pixMap, 0, 0, pixMap->Contents.Width, pixMap->Contents.Height, color);
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
    if(w < 0 || h < 0 || x >= dst->Contents.Width || y >= dst->Contents.Height)
        return;

    int x2 = min(dst->Contents.Width, x + w);
    int y2 = min(dst->Contents.Height, y + h);
    if(x2 <= 0 || y2 <= 0 || sx >= src->Contents.Width || sy >= src->Contents.Height)
        return;

    int sx2 = min(src->Contents.Width, sx + w);
    int sy2 = min(src->Contents.Height, sy + h);

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
        int sx2b = sx + w + (x2 - (x + w));
        int sy2b = sy + h + (y2 - (y + h));
        for(int Y = y2 - 1, sY = sy2b - 1; Y >= y && sY >= sy; --Y, --sY)
        {
            for(int X = x2 - 1, sX = sx2b - 1; X >= x && sX >= sx; --X, --sX)
                pmSetPixel(dst, X, Y, pmGetPixel(src, sX, sY));
        }
    }
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
    if(w < 0 || h < 0 || x >= dst->Contents.Width || y >= dst->Contents.Height)
        return;

    int x2 = min(dst->Contents.Width, x + w);
    int y2 = min(dst->Contents.Height, y + h);
    if(x2 <= 0 || y2 <= 0 || sx >= src->Contents.Width || sy >= src->Contents.Height)
        return;

    int sx2 = min(src->Contents.Width, sx + w);
    int sy2 = min(src->Contents.Height, sy + h);

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
        int sx2b = sx + w + (x2 - (x + w));
        int sy2b = sy + h + (y2 - (y + h));
        for(int Y = y2 - 1, sY = sy2b - 1; Y >= y && sY >= sy; --Y, --sY)
        {
            for(int X = x2 - 1, sX = sx2b - 1; X >= x && sX >= sx; --X, --sX)
                pmSetPixel(dst, X, Y, pmBlendPixel(pmGetPixel(dst, X, Y), pmGetPixel(src, sX, sY)));
        }
    }
}

void pmDrawFrame(struct pmPixMap *pixMap, int x, int y, int w, int h, int sunken)
{
    union pmColor tl = sunken ? pmColorDarkGray : pmColorWhite;
    union pmColor br = sunken ? pmColorWhite : pmColorDarkGray;

    pmHLine(pixMap, x, y + h - 1, x + w - 1, br);
    pmVLine(pixMap, x + w - 1, y, y + h - 1, br);
    pmHLine(pixMap, x, y, x + w - 1, tl);
    pmVLine(pixMap, x, y, y + h - 1, tl);
}

void pmInvalidate(struct pmPixMap *pixMap, int x, int y, int w, int h)
{
    struct wmRectangle rect = { x, y, w, h };
    pmInvalidateRect(pixMap, rect);
}

void pmInvalidateRect(struct pmPixMap *pixMap, struct wmRectangle rect)
{
    if(!pixMap) return;
    if(pixMap->Parent)
    {
        rect.X += pixMap->Parent->Contents.X;
        rect.Y += pixMap->Parent->Contents.Y;
        pmInvalidateRect(pixMap->Parent, rect);
        return;
    }
    pixMap->Dirty = wmRectangleAdd(pixMap->Dirty, rect);
}

void pmInvalidateWhole(struct pmPixMap *pixMap)
{
    pixMap->Dirty = pixMap->Contents;
}

struct wmRectangle pmGetDirtyRectangle(struct pmPixMap *pixMap)
{
    if(!pixMap) return wmRectangleEmpty;
    return pixMap->Dirty;
}

struct wmRectangle pmGetAndClearDirtyRectangle(struct pmPixMap *pixMap)
{
    struct wmRectangle dirty = pmGetDirtyRectangle(pixMap);
    pmClearDirty(pixMap);
}

void pmClearDirty(struct pmPixMap *pixMap)
{
    if(!pixMap) return;
    pixMap->Dirty = wmRectangleEmpty;
}

void pmDelete(struct pmPixMap *pixMap)
{
    if(!pixMap) return;
    if(pixMap->ReleasePixels && pixMap->Pixels)
        free(pixMap->Pixels);
    free(pixMap);
}
