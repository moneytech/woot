#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <woot/font.h>
#include <woot/pixmap.h>
#include <woot/wm.h>

#include <../internal/syscall.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

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

int wmInitialize()
{
    titleFont = fntLoad("WOOT_OS:/test.ttf");
    //if(titleFont) fntSetPixelSize(titleFont, 16);
    if(titleFont) fntSetPointSize(titleFont, 12, 96);
    backColor = pmColorGray;
    titleForeground = pmColorWhite;
    titleBackground = pmColorBlue;
    return 0;
}

struct wmWindow *wmCreateWindow(int x, int y, int width, int height, const char *title, int decorate)
{
    struct wmWindow *wnd = (struct wmWindow *)calloc(1, sizeof(struct wmWindow));
    errno = ENOMEM;
    if(!wnd) return NULL;

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
        errno = -ENOMEM;
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
        errno = -ENOMEM;
        wmDeleteWindow(wnd);
        return NULL;
    }

    pmFillRectangle(wnd->Contents, 0, 0, width, height, backColor);

    if(decorate)
    {
        struct wmRectangle dragRect = { 2, 2, width - 4, 24 };
        wmSetDragRectangle(wnd, &dragRect);
        wmDecorateWindow(wnd);
    }

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
    return syscall1(SYS_update_window, window->ID);
}

int wmUpdateWindowByID(int id)
{
    return syscall1(SYS_update_window, id);
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
    pmDrawFrame(window->Contents, 0, 0, window->Contents->Width, window->Contents->Height, 0);
    pmFillRectangle(window->Contents, 2, 2, window->Contents->Width - 4, 24, titleBackground);
    if(titleFont) fntDrawString(titleFont, window->Contents, 8, 19, window->Name, titleForeground);
    pmDrawFrame(window->Contents, 2, 2, window->Contents->Width - 4, 24, 1);
}

void wmDeleteWindow(struct wmWindow *window)
{
    if(!window) return;
    if(window->Name) free(window->Name);
    if(window->ClientArea) pmDelete(window->ClientArea);
    if(window->Contents)
    {
        int pagesize = getpagesize();
        munmap(window->Contents->Pixels, pagesize * ((window->Contents->Pitch * window->Contents->Height) + pagesize - 1));
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
    return titleFont;
}

int wmGetEvent(int window, struct wmEvent *event)
{
    int res = syscall2(SYS_get_event, window, (long)event);
}

int wmPeekEvent(int window, struct wmEvent *event, int remove)
{
    int res = syscall3(SYS_peek_event, window, (long)event, remove);
    if(res < 0) return res;
}

int wmProcessEvent(struct wmWindow *window, struct wmEvent *event)
{
    if(!window || !event) return -EINVAL;
    if(event->Type == ET_MOUSE)
    {   // fixup coordinates for decorated windows
        event->Mouse.X -= window->ClientRectangle.X;
        event->Mouse.Y -= window->ClientRectangle.Y;
    }
}

void wmCleanup()
{
    fntDelete(titleFont);
}
