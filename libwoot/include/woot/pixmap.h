#ifndef PIXMAP_H
#define PIXMAP_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct pmPixelFormat
{
    int BPP;
    int AlphaShift, RedShift, GreenShift, BlueShift;
    int AlphaBits, RedBits, GreenBits, BlueBits;
};

union pmColor
{
    struct
    {
        unsigned char A;
        unsigned char R;
        unsigned char G;
        unsigned char B;
    };
    unsigned int Value;
};


extern union pmColor pmColorBlack;
extern union pmColor pmColorBlue;
extern union pmColor pmColorGreen;
extern union pmColor pmColorCyan;
extern union pmColor pmColorRed;
extern union pmColor pmColorMagenta;
extern union pmColor pmColorBrown;
extern union pmColor pmColorGray;
extern union pmColor pmColorDarkGray;
extern union pmColor pmColorBrightBlue;
extern union pmColor pmColorBrightGreen;
extern union pmColor pmColorBrightCyan;
extern union pmColor pmColorBrightRed;
extern union pmColor pmColorBrightMagenta;
extern union pmColor pmColorYellow;
extern union pmColor pmColorWhite;
extern union pmColor pmColorTransparent;

struct pmPixMap;

int pmFormatEqual(struct pmPixelFormat a, struct pmPixelFormat b);
unsigned int pmFormatPixelsToBytes(struct pmPixelFormat format, unsigned int pixels);

union pmColor pmColorFromRGB(unsigned char r, unsigned char g, unsigned char b);
union pmColor pmColorFromARGB(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
unsigned int pmColorToValue(union pmColor color, struct pmPixelFormat format);
union pmColor pmColorFromValue(struct pmPixelFormat format, unsigned int value);
union pmColor pmColorFromIndex(struct pmPixMap *pixMap, unsigned int index);
unsigned int pmIndexFromColor(struct pmPixMap *pixMap, union pmColor color);

union pmColor *pmPaletteCreate(struct pmPixelFormat *format);
void pmPaletteDelete(union pmColor *palette);

struct pmPixMap *pmCreate(int width, int height, struct pmPixelFormat format);
struct pmPixMap *pmCreate2(int width, int height, int pitch, struct pmPixelFormat format, void *pixels, int releasePixels);
struct pmPixMap *pmFromPixMap(struct pmPixMap *src, struct pmPixelFormat format);
struct pmPixMap *pmSubPixMap(struct pmPixMap *src, int x, int y, int w, int h);
struct pmPixMap *pmLoadPNG(const char *filename);
struct wmRectangle pmGetRectangle(struct pmPixMap *pixMap);
int pmGetPitch(struct pmPixMap *pixMap);
void *pmGetPixels(struct pmPixMap *pixMap);
void pmSetPixel(struct pmPixMap *pixMap, int x, int y, union pmColor color);
union pmColor pmGetPixel(struct pmPixMap *pixMap, int x, int y);
union pmColor pmBlendPixel(union pmColor a, union pmColor b);
void pmHLine(struct pmPixMap *pixMap, int x1, int y, int x2, union pmColor c);
void pmVLine(struct pmPixMap *pixMap, int x, int y1, int y2, union pmColor c);
void pmVFlip(struct pmPixMap *pixMap);
void pmLine(struct pmPixMap *pixMap, int x1, int y1, int x2, int y2, union pmColor c);
void pmRectangle(struct pmPixMap *pixMap, int x, int y, int w, int h, union pmColor c);
void pmRectangleRect(struct pmPixMap *pixMap, struct wmRectangle rect, union pmColor c);
void pmFillRectangle(struct pmPixMap *pixMap, int x, int y, int w, int h, union pmColor c);
void pmClear(struct pmPixMap *pixMap, union pmColor color);
void pmBlit(struct pmPixMap *dst, struct pmPixMap *src, int sx, int sy, int x, int y, int w, int h);
void pmAlphaBlit(struct pmPixMap *dst, struct pmPixMap *src, int sx, int sy, int x, int y, int w, int h);
void pmDrawFrame(struct pmPixMap *pixMap, int x, int y, int w, int h, int sunken);
void pmInvalidate(struct pmPixMap *pixMap, int x, int y, int w, int h);
void pmInvalidateRect(struct pmPixMap *pixMap, struct wmRectangle rect);
void pmInvalidateWhole(struct pmPixMap *pixMap);
struct wmRectangle pmGetDirtyRectangle(struct pmPixMap *pixMap);
struct wmRectangle pmGetAndClearDirtyRectangle(struct pmPixMap *pixMap);
void pmClearDirty(struct pmPixMap *pixMap);
void pmDelete(struct pmPixMap *pixMap);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PIXMAP_H
