#ifndef KWM_H
#define KWM_H

#include <inputdevice.h>
#include <list.h>
#include <messagequeue.h>
#include <mutex.h>
#include <pixmap.h>
#include <sequencer.h>
#include <types.h>

class FrameBuffer;
class Semaphore;
class Thread;

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

    struct wmEvent
    {
        int Type;
        union
        {
            struct
            {
                int Data[7];
            } Other;
            struct
            {
                int Key;
                int Flags;
            } Keyboard;
            struct
            {
                int X, Y;
                int DeltaX, DeltaY;
                int ButtonsPressed;
                int ButtonsReleased;
                int ButtonsHeld;
            } Mouse;
        };
    };
    // end of user mode structs/unions

    struct Point
    {
        int X, Y;
        Point();
        Point(int x, int y);
        bool IsInside(Rectangle rect);
        Point operator +(Point p);
        Point operator -(Point p);
        Point operator +=(Point p);
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

        Window(WindowManager *WM, int x, int y, int w, int h, PixMap::PixelFormat *fmt);
        ~Window();
    public:
        int ID;
        Thread *Owner;
        WindowManager *Manager;
        Point Position;
        PixMap *Contents;
        Rectangle Dirty;
        bool Visible;
        bool UseAlpha;
        MessageQueue<InputDevice::Event> Events;
        Rectangle DragRectangle;

        void Invalidate_nolock();
        void Invalidate();
        Rectangle ToRectangle();
        void Update_nolock();
        void Update();
    };
private:

    FrameBuffer *fb;
    PixMap backBuffer;
    List<Window *> windows;
    Mutex lock;
    Point mousePos;
    Point mouseHotspot;
    int mouseWndId;
    int activeWindowId = 0;
    int dragWindowId = -1;
    Semaphore *inputThreadFinished;
    Thread *thread;
    Rectangle desktopRect;
    bool drag;
    Point dragPoint;

    static int inputThread(uintptr_t arg);
    WindowManager(FrameBuffer *fb);
public:
    static WindowManager *WM;

    static void Initialize(FrameBuffer *fb);
    static void DestroyThreadWindows_nolock(Thread *thread);
    static void DestroyThreadWindows(Thread *thread);
    static Window *GetByID_nolock(int id);
    static Window *GetByID(int id);
    static int CreateWindow_nolock(int x, int y, int w, int h, PixMap::PixelFormat *fmt);
    static int CreateWindow(int x, int y, int w, int h, PixMap::PixelFormat *fmt);
    static bool DestroyWindow_nolock(int id);
    static bool DestroyWindow(int id);
    static bool ShowWindow_nolock(int id);
    static bool ShowWindow(int id);
    static bool HideWindow_nolock(int id);
    static bool HideWindow(int id);
    static bool BringWindowToFront_nolock(int id);
    static bool BringWindowToFront(int id);
    static bool GetWindowPosition_nolock(int id, Point *pt);
    static bool GetWindowPosition(int id, Point *pt);
    static bool SetWindowPosition_nolock(int id, int x, int y);
    static bool SetWindowPosition(int id, int x, int y);
    static bool DrawRectangle_nolock(int id, Rectangle rect, PixMap::Color color);
    static bool DrawRectangle(int id, Rectangle rect, PixMap::Color color);
    static bool DrawFilledRectangle_nolock(int id, Rectangle rect, PixMap::Color color);
    static bool DrawFilledRectangle(int id, Rectangle rect, PixMap::Color color);
    static bool UpdateWindow_nolock(int id);
    static bool UpdateWindow(int id);
    static bool RedrawWindow_nolock(int id);
    static bool RedrawWindow(int id);
    static bool DrawLine_nolock(int id, int x1, int y1, int x2, int y2, PixMap::Color color);
    static bool DrawLine(int id, int x1, int y1, int x2, int y2, PixMap::Color color);
    static bool Blit_nolock(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h);
    static bool Blit(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h);
    static bool AlphaBlit_nolock(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h);
    static bool AlphaBlit(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h);
    static bool InvalidateRectangle_nolock(int id, Rectangle &rect);
    static bool InvalidateRectangle(int id, Rectangle &rect);
    static void SetMousePosition_nolock(Point pos);
    static void SetMousePosition(Point pos);
    static Point GetMousePosition_nolock();
    static Point GetMousePosition();
    static bool PutEvent_nolock(int id, InputDevice::Event event);
    static bool PutEvent(int id, InputDevice::Event event);
    static bool SetDragRectangle_nolock(int id, Rectangle rect);
    static bool SetDragRectangle(int id, Rectangle rect);
    static InputDevice::Event GetEvent_nolock(int id);
    static InputDevice::Event GetEvent(int id);
    static InputDevice::Event PeekEvent_nolock(int id, bool remove);
    static InputDevice::Event PeekEvent(int id, bool remove);
    static Rectangle GetWindowRectangle_nolock(int id);
    static Rectangle GetWindowRectangle(int id);
    static void Cleanup();
};

#endif // KWM_H
