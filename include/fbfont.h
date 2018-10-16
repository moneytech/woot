#ifndef FBFONT_H
#define FBFONT_H

#include <types.h>

#define FONT_GLYPHS 256
#define FONT_SCANLINES 16
#define FONT_BITS 8

extern byte fbFont[FONT_GLYPHS][FONT_SCANLINES];

#endif // FBFONT_H
