#ifndef _WOOT_H
#define _WOOT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct wmRectangle
{
    int X, Y, Width, Height;
};

int wmCreateWindow(int x, int y, int width, int height);
int wmShowWindow(int window);
int wmHideWindow(int window);
int wmDestroyWindow(int window);
int wmDrawRectangle(int window, struct wmRectangle *rect, int color);
int wmDrawFilledRectangle(int window, struct wmRectangle *rect, int color);
int wmUpdateWindow(int window);
int wmRedrawWindow(int window);
int wmDrawLine(int window, int x1, int y1, int x2, int y2, int color);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _WOOT_H
