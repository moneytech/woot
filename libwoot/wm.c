#include <stdlib.h>
#include <sys/syscall.h>
#include <woot/pixmap.h>
#include <woot/wm.h>

#include <../internal/syscall.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

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

int wmCreateWindow(int x, int y, int width, int height)
{
    int wnd = syscall4(SYS_create_window, x, y, width, height);
    if(wnd < 0) return wnd;
    return wnd;
}

int wmShowWindow(int window)
{
    return syscall1(SYS_show_window, window);
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

int wmUpdateWindow(int window)
{
    return syscall1(SYS_update_window, window);
}

int wmRedrawWindow(int window)
{
    return syscall1(SYS_redraw_window, window);
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

int wmInvalidateRectangle(int window, struct wmRectangle *rect)
{
    if(!rect)
    {
        int w, h;
        int r = wmGetWindowSize(window, &w, &h);
        if(r < 0) return r;
        struct wmRectangle rect = { 0, 0, w, h };
        return syscall2(SYS_draw_rectangle, window, (long)&rect);
    }
    return syscall2(SYS_draw_rectangle, window, (long)rect);
}

int wmInvalidate(int window, int x, int y, int w, int h)
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
