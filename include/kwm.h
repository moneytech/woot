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

    // start of user mode structs/unions
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

    struct pmPixelFormat
    {
        int BPP;
        int AlphaShift, RedShift, GreenShift, BlueShift;
        int AlphaBits, RedBits, GreenBits, BlueBits;
    };

    union pmColor
    {
        unsigned int Value;
        struct
        {
            unsigned char A;
            unsigned char R;
            unsigned char G;
            unsigned char B;
        };
    };

    struct pmPixMap
    {
        int Width, Height;
        int Pitch;
        struct pmPixelFormat Format;
        int ReleasePixels;
        union
        {
            void *Pixels;
            unsigned char *PixelBytes;
        };
    };
    // end of user mode structs/unions

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
    static bool Blit(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h);
    static bool AlphaBlit(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h);
    static void Cleanup();

private:
    Window *getByID(int id);
    void RedrawAll();
};

#endif // KWM_H
