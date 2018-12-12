#ifndef FONT_H
#define FONT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct fntFont;
struct pmPixMap;
union pmColor;

struct fntFont *fntLoad(const char *filename);
int fntSetPixelSize(struct fntFont *font, int size);
int fntSetPointSize(struct fntFont *font, double size, int dpi);
float fntDrawCharacter(struct fntFont *font, struct pmPixMap *pixMap, int x, int y, int chr, union pmColor color);
float fntMeasureCharacter(struct fntFont *font, int chr);
float fntDrawString(struct fntFont *font, struct pmPixMap *pixMap, int x, int y, const char *str, union pmColor color);
float fntMeasureString(struct fntFont *font, const char *str);
float fntGetPixelAscender(struct fntFont *font);
float fntGetPixelHeight(struct fntFont *font);
void fntDelete(struct fntFont *font);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FONT_H
