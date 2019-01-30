#include "SDL_video.h"
#include "../SDL_sysvideo.h"

#include "SDL_wootVideo.h"
#include "SDL_wootEvents.h"

#include <woot/pixmap.h>
#include <woot/ui.h>
#include <woot/wm.h>

#define WOOTVID_DRIVER_NAME "woot"

static int WOOT_Available(void);
static SDL_VideoDevice *WOOT_CreateDevice(int devindex);
static int WOOT_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Surface *WOOT_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static SDL_Rect **WOOT_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static void WOOT_UpdateRects(_THIS, int numrects, SDL_Rect *rects);
static void WOOT_SetCaption(_THIS, const char *title, const char *icon);
static int WOOT_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void WOOT_DeleteDevice(_THIS);
static void WOOT_VideoQuit(_THIS);

VideoBootStrap WOOT_bootstrap =
{
    WOOTVID_DRIVER_NAME, "SDL WOOT video driver",
    WOOT_Available, WOOT_CreateDevice
};

static int WOOT_Available(void)
{
    return 1;
}

static SDL_VideoDevice *WOOT_CreateDevice(int devindex)
{
    _THIS = (SDL_VideoDevice *)SDL_calloc(1, sizeof(SDL_VideoDevice));
    if(!this) return NULL;

    this->hidden = (struct SDL_PrivateVideoData *)SDL_calloc(1, sizeof(struct SDL_PrivateVideoData));
    if(!this->hidden)
    {
        SDL_free(this);
        return NULL;
    }

    this->free = WOOT_DeleteDevice;
    this->VideoInit = WOOT_VideoInit;
    this->ListModes = WOOT_ListModes;
    this->UpdateRects = WOOT_UpdateRects;
    this->SetVideoMode = WOOT_SetVideoMode;
    this->SetCaption = WOOT_SetCaption;
    this->SetColors = WOOT_SetColors;
    this->VideoQuit = WOOT_VideoQuit;
    this->PumpEvents = WOOT_PumpEvents;
    this->InitOSKeymap = WOOT_InitOSKeymap;
    return this;
}

static int WOOT_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
    vformat->BitsPerPixel = 32;
    vformat->BytesPerPixel = vformat->BitsPerPixel << 3;
    return 0;
}

static SDL_Rect **WOOT_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
     return (SDL_Rect **)-1;
}

static SDL_Surface *WOOT_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags)
{
    // destroy possibly existing previous structures
    if(this->hidden->pm)
    {
        pmDelete(this->hidden->pm);
        this->hidden->pm = NULL;
    }
    if(this->hidden->window)
    {
        wmDeleteWindow(this->hidden->window);
        this->hidden->window = NULL;
    }
    if(current && current->format && current->format->palette)
    {
        if(current->format->palette->colors)
        {
            free(current->format->palette->colors);
            current->format->palette->colors = NULL;
        }
        free(current->format->palette);
        current->format->palette = NULL;
    }

    // determine pixel format
    struct pmPixelFormat pf;
    memset(&pf, 0, sizeof(pf));
    switch(bpp)
    {
    case 8:
        pf.BPP = 8;
        break;
    case 15:
        pf.BPP = 15;
        pf.RedBits = 5;
        pf.GreenBits = 5;
        pf.BlueBits = 5;
        pf.RedShift = 10;
        pf.GreenShift = 5;
        pf.BlueShift = 0;
        break;
    case 16:
        pf.BPP = 15;
        pf.RedBits = 5;
        pf.GreenBits = 6;
        pf.BlueBits = 5;
        pf.RedShift = 11;
        pf.GreenShift = 6;
        pf.BlueShift = 0;
        break;
    case 24:
    case 32:
        pf.BPP = bpp;
        pf.RedBits = 8;
        pf.GreenBits = 8;
        pf.BlueBits = 8;
        pf.RedShift = 16;
        pf.GreenShift = 8;
        pf.BlueShift = 0;
        break;
    default:
        SDL_SetError("SDL_SetVideoMode(%d, %d, %d, 0x%.8x): not supported bpp", width, height, bpp, flags);
        return NULL;
    }

    // create main pixmap
    this->hidden->pm = pmCreate(width, height, pf);
    if(!this->hidden->pm)
        return NULL;

    // fill more pixel format structures
    current->format->BitsPerPixel = bpp;
    current->format->BytesPerPixel = (bpp + 7) / 8;
    current->format->alpha = 255;
    if(bpp > 8)
    {
        current->format->Amask = 0;
        current->format->Ashift = 0;
        current->format->Rshift = pf.GreenBits + pf.BlueBits;
        current->format->Gshift = pf.BlueBits;
        current->format->Bshift = 0;
        current->format->Rmask = ((1 << pf.RedBits) - 1) << current->format->Rshift;
        current->format->Gmask = ((1 << pf.GreenBits) - 1) << current->format->Gshift;
        current->format->Bmask = ((1 << pf.BlueBits) - 1) << current->format->Bshift;
    }
    else
    {
        // create palette
        current->format->palette = (SDL_Palette *)calloc(1, sizeof(SDL_Palette));
        current->format->palette->ncolors = 1 << bpp;
        current->format->palette->colors = (SDL_Color *)calloc(current->format->palette->ncolors, sizeof(SDL_Color));

        current->format->Amask = 0;
        current->format->Ashift = 0;
        current->format->Rmask = 0;
        current->format->Rshift = 0;
        current->format->Gmask = 0;
        current->format->Gshift = 0;
        current->format->Bmask = 0;
        current->format->Bshift = 0;
    }

    // create main window
    struct wmWindow *wnd = wmCreateWindow(100, 100, wmGetDecoratedWidth(width), wmGetDecoratedHeight(height), "SDL window", 1);
    this->hidden->window = wnd;
    if(!wnd) return NULL;
    wmShowWindow(wnd);

    current->w = width;
    current->h = height;
    current->pitch = pmGetPitch(this->hidden->pm);
    current->pixels = pmGetPixels(this->hidden->pm);

    pmFillRectangle(this->hidden->pm, 0, 0, width, height, pmColorRed);
    wmRedrawWindow(wnd);

    return current;
}

static void WOOT_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
    struct wmWindow *wnd = (struct wmWindow *)this->hidden->window;
    if(!wnd || !this || !this->hidden || !this->hidden->pm)
        return;

    for(int i = 0; i < numrects; ++i)
    {
        SDL_Rect *r = rects + i;
        struct wmRectangle rect = { r->x, r->y, r->w, r->h };
        pmBlit(wnd->ClientArea, this->hidden->pm, rect.X, rect.Y, rect.X, rect.Y, rect.Width, rect.Height);
        pmInvalidateRect(wnd->ClientArea, rect);
    }

    //uiControlRedraw(wnd->RootControl);
    //wmUpdateWindow(wnd);
    wmRedrawWindow(wnd);
}

static void WOOT_SetCaption(_THIS, const char *title, const char *icon)
{
    if(!this || !this->hidden || !this->hidden->window)
        return;
    wmSetTitle(this->hidden->window, title);
}

static int WOOT_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
    if(!this || !this->hidden || !this->hidden->pm || !colors)
        return 0;
    struct pmPixMap *pm = this->hidden->pm;
    for(int i = 0; i < ncolors; ++i)
    {
        SDL_Color *c = colors + i;
        union pmColor color = pmColorFromRGB(c->r, c->g, c->b);
        pmSetPaletteEntry(pm, firstcolor + i, color);
    }
    return 1;
}

static void WOOT_DeleteDevice(_THIS)
{
    SDL_free(this->hidden);
    SDL_free(this);
}

static void WOOT_VideoQuit(_THIS)
{
    if(this && this->hidden)
    {
        struct wmWindow *wnd = this->hidden->window;
        if(wnd) wmDeleteWindow(wnd);
        struct pmPixMap *pm = this->hidden->pm;
        if(pm) pmDelete(pm);
        if(this->screen && this->screen->format && this->screen->format->palette)
        {
            if(this->screen->format->palette->colors)
                free(this->screen->format->palette->colors);
            free(this->screen->format->palette);
        }
    }
}
