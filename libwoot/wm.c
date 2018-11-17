#include <sys/syscall.h>
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
