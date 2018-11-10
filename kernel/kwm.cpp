#include <framebuffer.h>
#include <kwm.h>
#include <stdlib.h>

PixMap::PixelFormat WindowManager::DefaultPixelFormat;
WindowManager *WindowManager::WM;
Sequencer<int> WindowManager::Window::ids(0);

WindowManager::WindowManager(FrameBuffer *fb) :
    fb(fb), backBuffer(fb->Pixels, fb->Pixels->Format)
{
    Window *desktop = new Window(this, 0, 0, fb->Pixels->Width, fb->Pixels->Height);
    windows.Prepend(desktop);
    desktop->Contents->Clear(PixMap::Color::DarkGray);
    Window *test1 = new Window(this, 100, 200, 700, 400);
    windows.Append(test1);
    Window *test2 = new Window(this, 500, 50, 480, 300);
    PixMap *logo = PixMap::Load("WOOT_OS:/logo.bmp");
    PixMap *ologo = logo;
    logo = new PixMap(logo, DefaultPixelFormat);
    delete ologo;
    test2->Contents->FillRectangle(0, 0, test2->Contents->Width, test2->Contents->Height, PixMap::Color::BrightBlue);
    test2->Contents->Rectangle(100 - 1, 100 - 1, logo->Width + 2, logo->Height + 2, PixMap::Color::White);
    test2->Contents->Blit(logo, 0, 0, 100, 100, logo->Width, logo->Height);
    delete logo;
    windows.Append(test2);

    desktop->Visible = true;
    test1->Visible = true;
    test2->Visible = true;
}

void WindowManager::Initialize(FrameBuffer *fb)
{
    DefaultPixelFormat = fb->Pixels->Format;
    WM = new WindowManager(fb);
}

void WindowManager::Cleanup()
{
    if(WM) delete WM;
}

WindowManager::Window *WindowManager::GetByID(int id)
{
    if(!lock.Acquire(0, false))
        return nullptr;
    Window *res = nullptr;
    for(Window *wnd : windows)
    {
        if(wnd->ID == id)
        {
            res = wnd;
            break;
        }
    }
    lock.Release();
    return res;
}

void WindowManager::RedrawAll()
{
    if(!lock.Acquire(0, false))
        return;
    for(Window *wnd : windows)
    {
        if(!wnd->Visible) continue;
        backBuffer.Blit(wnd->Contents, 0, 0, wnd->Position.X, wnd->Position.Y, wnd->Contents->Width, wnd->Contents->Height);
        backBuffer.Rectangle(wnd->Position.X, wnd->Position.Y, wnd->Contents->Width, wnd->Contents->Height, PixMap::Color::Cyan);
    }
    fb->Pixels->Blit(&backBuffer, 0, 0, 0, 0, backBuffer.Width, backBuffer.Height);
    lock.Release();
}

WindowManager::Window::Window(WindowManager *wm, int x, int y, int w, int h) :
    ID(ids.GetNext()),
    Manager(wm),
    Position(x, y),
    Contents(new PixMap(w, h, DefaultPixelFormat)),
    Dirty(0, 0, w, h)
{
}

WindowManager::Window::~Window()
{
    if(Contents) delete Contents;
}

void WindowManager::Window::Invalidate()
{
    if(!Manager || !Manager->lock.Acquire(0, false))
        return;
    Rectangle d(0, 0, Contents->Width, Contents->Height);
    Dirty.Add(d);
    Manager->lock.Release();
}

WindowManager::Rectangle WindowManager::Window::ToRectangle()
{
    return Rectangle(Position, Contents->Width, Contents->Height);
}

void WindowManager::Window::Update()
{
    if(!Manager || !Manager->lock.Acquire(0, false))
        return;

    // convert dirty rect to screen coordinates
    Rectangle updateRect = Dirty;
    updateRect.Origin.X += Position.X;
    updateRect.Origin.Y += Position.Y;

    bool skip = true;
    for(Window *wnd : Manager->windows)
    {
        if(wnd == this)
            skip = false;
        if(skip || !wnd->Visible)
            continue;
        Rectangle wndRect = wnd->ToRectangle();
        Rectangle isect = updateRect.Intersection(wndRect);
        if(!isect.Width || !isect.Height) continue;

        // convert isect to window coordinates
        Rectangle localRect = isect;
        localRect.Origin.X -= wnd->Position.X;
        localRect.Origin.Y -= wnd->Position.Y;

        Manager->backBuffer.Blit(wnd->Contents, localRect.Origin.X, localRect.Origin.Y, isect.Origin.X, isect.Origin.Y, isect.Width, isect.Height);
    }

    Dirty = Rectangle(0, 0, 0, 0);
    Manager->fb->Pixels->Blit(&Manager->backBuffer,
                              updateRect.Origin.X, updateRect.Origin.Y,
                              updateRect.Origin.X, updateRect.Origin.Y,
                              updateRect.Width, updateRect.Height);
    Manager->lock.Release();
}

WindowManager::Rectangle::Rectangle() :
    Origin(0, 0), Width(0), Height(0)
{
}

WindowManager::Rectangle::Rectangle(int x, int y, int w, int h) :
    Origin(x, y), Width(w), Height(h)
{
}

WindowManager::Rectangle::Rectangle(WindowManager::Point o, int w, int h) :
    Origin(o), Width(w), Height(h)
{
}

bool WindowManager::Rectangle::Contains(WindowManager::Point pt)
{
    return pt.IsInside(*this);
}

bool WindowManager::Rectangle::Intersects(WindowManager::Rectangle rect)
{
    return !((Origin.X + Width) < rect.Origin.X || (rect.Origin.X + rect.Width) < Origin.X ||
             (Origin.Y + Height) < rect.Origin.Y || (rect.Origin.Y + rect.Height) < Origin.Y);
}

WindowManager::Rectangle WindowManager::Rectangle::Intersection(WindowManager::Rectangle rect)
{
    int x = max(Origin.X, rect.Origin.X);
    int x2 = min(Origin.X + Width, rect.Origin.X + rect.Width);
    int y = max(Origin.Y, rect.Origin.Y);
    int y2 = min(Origin.Y + Height, rect.Origin.Y + rect.Height);
    int w = x2 - x;
    int h = y2 - y;
    if(w <= 0 || h <= 0)
        return Rectangle();
    return Rectangle(x, y, w, h);
}

void WindowManager::Rectangle::Add(WindowManager::Rectangle rect)
{
    if(rect.Width <= 0 || rect.Height <= 0)
        return;
    if(Width <= 0 || Height <= 0)
    {
        Origin = rect.Origin;
        Width = rect.Width;
        Height = rect.Height;
        return;
    }
    int minx = min(Origin.X, rect.Origin.X);
    int miny = min(Origin.Y, rect.Origin.Y);
    int maxx = max(Origin.X + Width, rect.Origin.X + rect.Width);
    int maxy = max(Origin.Y + Height, rect.Origin.Y + rect.Height);
    Origin.X = minx;
    Origin.Y = miny;
    Width = maxx - minx;
    Height = maxy - miny;
}

WindowManager::Point::Point(int x, int y) :
    X(x), Y(y)
{
}

bool WindowManager::Point::IsInside(WindowManager::Rectangle rect)
{
    int x2 = rect.Origin.X + rect.Width;
    int y2 = rect.Origin.Y + rect.Height;
    return X >= rect.Origin.X && Y >= rect.Origin.Y && X <= x2 && Y <= y2;
}
