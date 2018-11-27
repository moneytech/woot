#include <errno.h>
#include <ft2build.h>
#include <woot/font.h>
#include <woot/pixmap.h>

#include FT_FREETYPE_H

struct fntFont
{
    FT_Face face;
};

static FT_Library library;

static int fntInitialize()
{
    if(!library)
    {
        FT_Error error = FT_Init_FreeType(&library);
        if(error)
        {
            library = NULL;
            return -EINVAL;
        }
    }
    return 0;
}

struct fntFont *fntLoad(const char *filename)
{
    fntInitialize();
    struct fntFont *font = (struct fntFont *)calloc(1, sizeof(struct fntFont));
    if(!font) return NULL;
    FT_Error error = FT_New_Face(library, filename, 0, &font->face);
    if(error)
    {
        font->face = NULL; // make sure it's NULL
        errno = EINVAL;
        fntDelete(font);
        return NULL;
    }
    return font;
}

int fntSetPixelSize(struct fntFont *font, int size)
{
    FT_Error error = FT_Set_Pixel_Sizes(font->face, 0, size);
    return error ? -EINVAL : 0;
}

int fntSetPointSize(struct fntFont *font, double size, int dpi)
{
    FT_Error error = FT_Set_Char_Size(font->face, 0, size * 64, 0, dpi);
    return error ? -EINVAL : 0;
}

int fntDrawCharacter(struct fntFont *font, struct pmPixMap *pixMap, int x, int y, int chr, union pmColor color)
{
    FT_Error error = FT_Load_Char(font->face, chr, FT_LOAD_DEFAULT);
    if(error) return -EINVAL;
    error = FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL);
    if(error) return -EINVAL;
    for(int i = 0; i < font->face->glyph->bitmap.rows; ++i)
    {
        unsigned char *line = (unsigned char *)(font->face->glyph->bitmap.buffer + i * font->face->glyph->bitmap.pitch);
        for(int j = 0; j < font->face->glyph->bitmap.width; ++j)
        {
            int _x = x + j + font->face->glyph->bitmap_left;
            int _y = y + i - font->face->glyph->bitmap_top;
            pmSetPixel(pixMap, _x, _y, pmBlendPixel(pmGetPixel(pixMap, _x, _y), pmColorFromARGB(line[j], color.R, color.G, color.B)));
        }
    }
    return font->face->glyph->advance.x >> 6;
}

int fntMeasureCharacter(struct fntFont *font, int chr)
{
    FT_Error error = FT_Load_Char(font->face, chr, FT_LOAD_ADVANCE_ONLY);
    if(error) return -EINVAL;
    return font->face->glyph->advance.x >> 6;
}

int fntDrawString(struct fntFont *font, struct pmPixMap *pixMap, int x, int y, const char *str, union pmColor color)
{
    while(*str)
    {
        int dx = fntDrawCharacter(font, pixMap, x, y, *str++, color);
        if(dx < 0) return dx;
        x += dx;
    }
    return x;
}

int fntMeasureString(struct fntFont *font, const char *str)
{
    int x = 0;
    while(*str)
    {
        int dx = fntMeasureCharacter(font, *str++);
        if(dx < 0) return dx;
        x += dx;
    }
    return x;
}

void fntDelete(struct fntFont *font)
{
    if(!font) return;
    if(font->face)
        FT_Done_Face(font->face);
}
