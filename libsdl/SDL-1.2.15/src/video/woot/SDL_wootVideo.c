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
    if(bpp != 32)
    {
        SDL_SetError("SDL_SetVideoMode(%d, %d, %d, 0x%.8x) failed", width, height, bpp, flags);
        return NULL;
    }

    if(this->hidden->window)
    {
        wmDeleteWindow(this->hidden->window);
        this->hidden->window = NULL;
    }

    struct wmWindow *wnd = wmCreateWindow(100, 100, wmGetDecoratedWidth(width), wmGetDecoratedHeight(height), "SDL window", 1);
    if(!wnd) return NULL;
    wmShowWindow(wnd);

    this->hidden->window = wnd;
    struct pmPixMap *pm = uiControlGetPixMap(wnd->RootControl);
    uiControlSetBackColor(wnd->RootControl, pmColorTransparent);
    current->w = width;
    current->h = height;
    current->pitch = pmGetPitch(pm);
    current->pixels = pmGetPixels(pm);

    pmFillRectangle(pm, 0, 0, width, height, pmColorBlack);
    wmRedrawWindow(wnd);

    return current;
}

static void WOOT_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
    struct wmWindow *wnd = (struct wmWindow *)this->hidden->window;
    if(!wnd || !wnd->RootControl)
        return;

    struct pmPixMap *pm = uiControlGetPixMap(wnd->RootControl);
    for(int i = 0; i < numrects; ++i)
    {
        SDL_Rect *r = rects + i;
        struct wmRectangle rect = { r->x, r->y, r->w, r->h };
        pmInvalidateRect(pm, rect);
    }
    uiControlRedraw(wnd->RootControl);
    wmUpdateWindow(wnd);
}

static void WOOT_SetCaption(_THIS, const char *title, const char *icon)
{
    if(!this || !this->hidden || !this->hidden->window)
        return;
    wmSetTitle(this->hidden->window, title);
}

static void WOOT_DeleteDevice(_THIS)
{
    SDL_free(this->hidden);
    SDL_free(this);
}

static void WOOT_VideoQuit(_THIS)
{
    if(this->hidden)
    {
        struct wmWindow *wnd = this->hidden->window;
        if(wnd) wmDeleteWindow(wnd);
    }
}
