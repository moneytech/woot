#ifndef KWM_H
#define KWM_H

#include <list.h>
#include <mutex.h>
#include <pixmap.h>
#include <sequencer.h>
#include <types.h>

class FrameBuffer;

class WindowManager
{
public:
    static PixMap::PixelFormat DefaultPixelFormat;
    struct Rectangle;

    // user mode rectangle structure
    struct wmRectangle
    {
        int X, Y, Width, Height;
    };

    struct Point
    {
        int X, Y;
        Point(int x, int y);
        bool IsInside(Rectangle rect);
    };

    struct Rectangle
    {
        Point Origin;
        int Width, Height;
        Rectangle();
        Rectangle(int x, int y, int w, int h);
        Rectangle(Point o, int w, int h);
        bool Contains(Point pt);
        bool Intersects(Rectangle rect);
        Rectangle Intersection(Rectangle rect);
        void Add(Rectangle rect);
    };

    class Window
    {
        friend class WindowManager;
        static Sequencer<int> ids;

        Window(WindowManager *WM, int x, int y, int w, int h);
        ~Window();
    public:
        int ID;
        WindowManager *Manager;
        Point Position;
        PixMap *Contents;
        Rectangle Dirty;
        bool Visible;
        void Invalidate();
        Rectangle ToRectangle();
        void Update();
    };
private:

    FrameBuffer *fb;
    PixMap backBuffer;
    List<Window *> windows;
    Mutex lock;

    WindowManager(FrameBuffer *fb);
public:
    static WindowManager *WM;

    static void Initialize(FrameBuffer *fb);
    static Window *GetByID(int id);
    static int CreateWindow(int x, int y, int w, int h);
    static bool DestroyWindow(int id);
    static bool ShowWindow(int id);
    static bool HideWindow(int id);
    static bool BringWindowToFront(int id);
    static bool SetWindowPosition(int id, int x, int y);
    static bool DrawRectangle(int id, Rectangle rect, PixMap::Color color);
    static bool DrawFilledRectangle(int id, Rectangle rect, PixMap::Color color);
    static bool UpdateWindow(int id);
    static bool RedrawWindow(int id);
    static bool DrawLine(int id, int x1, int y1, int x2, int y2, PixMap::Color color);
    static void Cleanup();

private:
    Window *getByID(int id);
    void RedrawAll();
};

#endif // KWM_H
