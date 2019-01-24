#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <woot/font.h>
#include <woot/pixmap.h>
#include <woot/ui.h>
#include <woot/wm.h>

#include <../internal/syscall.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

static struct fntFont *defaultFont;
static struct fntFont *titleFont;
static union pmColor backColor;
static union pmColor titleForeground;
static union pmColor titleBackground;

struct wmRectangle wmRectangleEmpty = { 0, 0, 0, 0 };

struct wmRectangle wmRectangleAdd(struct wmRectangle a, struct wmRectangle b)
{
    if(b.Width <= 0 || b.Height <= 0)
        return a;
    if(a.Width <= 0 || a.Height <= 0)
    {
        a.X = b.X;
        a.Y = b.Y;
        a.Width = b.Width;
        a.Height = b.Height;
        return a;
    }
    int minx = min(a.X, b.X);
    int miny = min(a.Y, b.Y);
    int maxx = max(a.X + a.Width, b.X + b.Width);
    int maxy = max(a.Y + a.Height, b.Y + b.Height);
    a.X = minx;
    a.Y = miny;
    a.Width = maxx - minx;
    a.Height = maxy - miny;
    return a;
}

struct wmRectangle wmRectangleIntersection(struct wmRectangle a, struct wmRectangle b)
{
    int x = max(a.X, b.X);
    int x2 = min(a.X + a.Width, b.X + b.Width);
    int y = max(a.Y, b.Y);
    int y2 = min(a.Y + a.Height, b.Y + b.Height);
    int w = x2 - x;
    int h = y2 - y;
    if(w <= 0 || h <= 0)
        return wmRectangleEmpty;
    a.X = x; a.Y = y; a.Width = w; a.Height = h;
    return a;
}

int wmRectangleContainsPoint(struct wmRectangle *rect, int x, int y)
{
    return x >= rect->X && y >= rect->Y && x < (rect->X + rect->Width) && y < (rect->Y + rect->Height);
}

int wmInitialize()
{
    defaultFont = fntLoad("WOOT_OS:/test.ttf");
    titleFont = fntLoad("WOOT_OS:/title.ttf");
    if(defaultFont) fntSetPointSize(defaultFont, 11, 96);
    if(titleFont) fntSetPointSize(titleFont, 11, 96);
    backColor = pmColorGray;
    titleForeground = pmColorWhite;
    titleBackground = pmColorBlue;
    return 0;
}

void wmGetDesktopWindow(struct wmWindow *window)
{
    memset(window, 0, sizeof(struct wmWindow));
}

struct wmWindow *wmCreateWindow(int x, int y, int width, int height, const char *title, int decorate)
{
    struct wmWindow *wnd = (struct wmWindow *)calloc(1, sizeof(struct wmWindow));
    if(!wnd)
    {
        errno = ENOMEM;
        return NULL;
    }

    wnd->Name = strdup(title ? title : "");
    if(!wnd->Name)
    {
        wmDeleteWindow(wnd);
        return NULL;
    }

    int res = syscall4(SYS_create_window, x, y, width, height);
    if(res < 0)
    {
        errno = -res;
        wmDeleteWindow(wnd);
        return NULL;
    }
    wnd->ID = res;

    wnd->Contents = wmWindowToPixMap(wnd->ID);
    if(!wnd->Contents)
    {
        errno = ENOMEM;
        wmDeleteWindow(wnd);
        return NULL;
    }

    wnd->ClientRectangle.X = decorate ? 2 : 0;
    wnd->ClientRectangle.Y = decorate ? 26 : 0;
    wnd->ClientRectangle.Width = decorate ? width - 4 : width;
    wnd->ClientRectangle.Height = decorate ? height - 28 : height;

    wnd->ClientArea = pmSubPixMap(wnd->Contents, wnd->ClientRectangle.X, wnd->ClientRectangle.Y, wnd->ClientRectangle.Width, wnd->ClientRectangle.Height);
    if(!wnd->ClientArea)
    {
        errno = ENOMEM;
        wmDeleteWindow(wnd);
        return NULL;
    }

    pmFillRectangle(wnd->Contents, 0, 0, width, height, backColor);

    if(decorate)
    {
        struct wmRectangle titleRect = { 2, 2, width - 4, 24 };
        wnd->TitleBar = uiControlCreate(NULL, 0, wnd->Contents, titleRect.X, titleRect.Y, titleRect.Width, titleRect.Height, NULL, NULL);
        if(wnd->TitleBar)
        {
            uiControlSetBackColor(wnd->TitleBar, pmColorBlue);
            titleRect.Width -= 24;
            struct uiLabel *label = uiLabelCreate(wnd->TitleBar, 0, 0, titleRect.Width, titleRect.Height, wnd->Name, titleFont, NULL);
            if(label) uiControlSetTextColor((struct uiControl *)label, pmColorWhite);
            struct uiButton *closeButton = uiButtonCreate(wnd->TitleBar, titleRect.X + titleRect.Width + 3, 4, 24 - 8, titleRect.Height - 8, "X", titleFont, NULL);
            uiControlSetBackColor((struct uiControl *)closeButton, pmColorGray);
        }
        wmSetDragRectangle(wnd, &titleRect);
        wmDecorateWindow(wnd);
    }

    struct wmRectangle rect = pmGetRectangle(wnd->ClientArea);
    wnd->RootControl = uiControlCreate(NULL, 0, wnd->ClientArea, 0, 0, rect.Width, rect.Height, NULL, NULL);
    if(!wnd->RootControl)
    {
        errno = ENOMEM;
        wmDeleteWindow(wnd);
        return NULL;
    }
    uiControlRedraw(wnd->RootControl);

    return wnd;
}

int wmShowWindow(struct wmWindow *window)
{
    return syscall1(SYS_show_window, window->ID);
}

int wmHideWindow(int window)
{
    return syscall1(SYS_hide_window, window);
}

int wmDestroyWindow(int window)
{
    return syscall1(SYS_destroy_window, window);
}

int wmDrawRectangle(int window, struct wmRectangle *rect, int color)
{
    return syscall3(SYS_draw_rectangle, window, (long)rect, color);
}

int wmDrawFilledRectangle(int window, struct wmRectangle *rect, int color)
{
    return syscall3(SYS_draw_filled_rectangle, window, (long)rect, color);
}

int wmUpdateWindow(struct wmWindow *window)
{
    struct wmRectangle dirty = pmGetDirtyRectangle(window->Contents);
    wmInvalidateRectangle(window, &dirty);
    pmClearDirty(window->Contents);
    return syscall1(SYS_update_window, window->ID);
}

int wmRedrawWindow(struct wmWindow *window)
{
    return syscall1(SYS_redraw_window, window->ID);
}

int wmDrawLine(int window, int x1, int y1, int x2, int y2, int color)
{
    return syscall6(SYS_draw_line, window, x1, y1, x2, y2, color);
}

int wmBlit(int window, struct pmPixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    struct wmBlitInfo bi = { sx, sy, x, y, w, h };
    return syscall3(SYS_blit, window, (long)src, (long)&bi);
}


int wmAlphaBlit(int window, struct pmPixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    struct wmBlitInfo bi = { sx, sy, x, y, w, h };
    return syscall3(SYS_alpha_blit, window, (long)src, (long)&bi);
}

int wmMapWindow(int window, void **result)
{
    return syscall2(SYS_map_window, window, (long)result);
}

int wmInvalidateRectangle(struct wmWindow *window, struct wmRectangle *rect)
{
    if(!rect)
    {
        int w, h;
        int r = wmGetWindowSize(window->ID, &w, &h);
        if(r < 0) return r;
        struct wmRectangle rect = { 0, 0, w, h };
        return syscall2(SYS_invalidate_rect, window->ID, (long)&rect);
    }
    return syscall2(SYS_invalidate_rect, window->ID, (long)rect);
}

int wmInvalidate(struct wmWindow *window, int x, int y, int w, int h)
{
    struct wmRectangle rect = { x, y, w, h };
    return wmInvalidateRectangle(window, &rect);
}

int wmGetWindowSize(int window, int *w, int *h)
{
    return syscall3(SYS_get_window_size, window, (long)w, (long)h);
}

int wmGetPixelFormat(int window, struct pmPixelFormat *format, int *pitch)
{
    return syscall3(SYS_get_pixel_format, window, (long)format, (long)pitch);
}

struct pmPixMap *wmWindowToPixMap(int window)
{
    int w, h;
    if(wmGetWindowSize(window, &w, &h) < 0)
        return NULL;
    struct pmPixelFormat fmt;
    int pitch;
    if(wmGetPixelFormat(window, &fmt, &pitch) < 0)
        return NULL;
    void *pixels = NULL;
    if(wmMapWindow(window, &pixels) < 0)
        return NULL;
    return pmCreate2(w, h, pitch, fmt, pixels, 0);
}

int wmDecorateWindow(struct wmWindow *window)
{
    struct wmRectangle rect = pmGetRectangle(window->Contents);
    pmDrawFrame(window->Contents, 0, 0, rect.Width, rect.Height, 0);
    if(window->TitleBar) uiControlRedraw(window->TitleBar);
}

void wmDeleteWindow(struct wmWindow *window)
{
    if(!window) return;
    if(window->RootControl) uiControlDelete(window->RootControl);

    if(window->Name) free(window->Name);
    if(window->ClientArea) pmDelete(window->ClientArea);
    if(window->Contents)
    {
        int pagesize = getpagesize();
        struct wmRectangle rect = pmGetRectangle(window->Contents);
        munmap(pmGetPixels(window->Contents), pagesize * ((pmGetPitch(window->Contents) * rect.Height) + pagesize - 1));
        pmDelete(window->Contents);
    }
    wmDestroyWindow(window->ID);
    free(window);
}

int wmSetDragRectangle(struct wmWindow *window, struct wmRectangle *rect)
{
    return syscall2(SYS_set_drag_rect, window->ID, (long)rect);
}

struct fntFont *wmGetDefaultFont()
{
    return defaultFont;
}

int wmGetEvent(struct wmWindow *window, struct wmEvent *event)
{
    if(!window || !event) return -EINVAL;
    return syscall2(SYS_get_event, window->ID, (long)event);
}

int wmPeekEvent(struct wmWindow *window, struct wmEvent *event, int remove)
{
    if(!window || !event) return -EINVAL;
    return syscall3(SYS_peek_event, window->ID, (long)event, remove);
}

int wmProcessEvent(struct wmWindow *window, struct wmEvent *event)
{
    if(!window || !event) return -EINVAL;
    if(event->Type == ET_MOUSE)
    {   // fixup coordinates for decorated windows
        event->Mouse.X -= window->ClientRectangle.X;
        event->Mouse.Y -= window->ClientRectangle.Y;
    }
    uiControlProcessEvent(window->RootControl, *event);
    wmUpdateWindow(window);
    return 0;
}

void wmCleanup()
{
    fntDelete(titleFont);
}
