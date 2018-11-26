#ifndef _WOOT_H
#define _WOOT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct wmRectangle
{
    int X, Y, Width, Height;
};

struct wmBlitInfo
{
    int SX, SY;
    int X, Y;
    int Width, Height;
};

struct wmWindow
{
    int ID;
    char *Name;
    struct pmPixMap *Contents;
    struct pmPixMap *ClientArea;
    struct wmRectangle ClientRectangle;
};

extern struct wmRectangle wmRectangleEmpty;
struct wmRectangle wmRectangleAdd(struct wmRectangle a, struct wmRectangle b);
struct wmRectangle wmRectangleIntersection(struct wmRectangle a, struct wmRectangle b);

// TODO: Cleanup this mess

int wmInitialize();
struct wmWindow *wmCreateWindow(int x, int y, int width, int height, const char *title, int decorate);
int wmShowWindow(struct wmWindow *window);
int wmHideWindow(int window);
int wmDestroyWindow(int window);
int wmDrawRectangle(int window, struct wmRectangle *rect, int color);
int wmDrawFilledRectangle(int window, struct wmRectangle *rect, int color);
int wmUpdateWindow(struct wmWindow *window);
int wmUpdateWindowByID(int id);
int wmRedrawWindow(struct wmWindow *window);
int wmDrawLine(int window, int x1, int y1, int x2, int y2, int color);
int wmBlit(int window, struct pmPixMap *src, int sx, int sy, int x, int y, int w, int h);
int wmAlphaBlit(int window, struct pmPixMap *src, int sx, int sy, int x, int y, int w, int h);
int wmMapWindow(int window, void **result); // maps window pixels to userspace
int wmInvalidateRectangle(struct wmWindow *window, struct wmRectangle *rect);
int wmInvalidate(struct wmWindow *window, int x, int y, int w, int h);
int wmGetWindowSize(int window, int *w, int *h);
int wmGetPixelFormat(int window, struct pmPixelFormat *format, int *pitch);
struct pmPixMap *wmWindowToPixMap(int window);
int wmDecorateWindow(struct wmWindow *window);
void wmDeleteWindow(struct wmWindow *window);
int wmSetDragRectangle(struct wmWindow *window, struct wmRectangle *rect);
void wmCleanup();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _WOOT_H
