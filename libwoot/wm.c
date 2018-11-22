#include <sys/syscall.h>
#include <woot/pixmap.h>
#include <woot/wm.h>

#include <../internal/syscall.h>

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

