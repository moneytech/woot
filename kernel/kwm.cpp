#include <errno.h>
#include <framebuffer.h>
#include <inputdevice.h>
#include <kwm.h>
#include <process.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sysdefs.h>
#include <thread.h>

PixMap::PixelFormat WindowManager::DefaultPixelFormat;
WindowManager *WindowManager::WM;
Sequencer<int> WindowManager::Window::ids(0);

int WindowManager::inputThread(uintptr_t arg)
{
    WindowManager *WM = (WindowManager *)arg;
    for(;;)
    {
        InputDevice::Event event = InputDevice::GetEvent(0);
        if(event.DeviceType == InputDevice::Type::Mouse)
        {
            if(WM->lock.Acquire(5000, false))
            {
                bool inhibitEvent = false;
                int dx = event.Mouse.Movement[0];
                int dy = event.Mouse.Movement[1];

                // mouse acceleration
                int d = dx * dx + dy * dy;
                if(d > 16)
                {
                    int a = d * 5;
                    int b = d * 3;
                    dx = (dx * a) / b;
                    dy = (dy * a) / b;
                }

                event.Mouse.Movement[0] = dx;
                event.Mouse.Movement[1] = dy;

                int mx = WM->mousePos.X + dx;
                int my = WM->mousePos.Y + dy;
                WM->mousePos.X = clamp(0, WM->desktopRect.Width - 1, mx);
                WM->mousePos.Y = clamp(0, WM->desktopRect.Height - 1, my);
                WM->SetMousePosition_nolock(WM->mousePos);

                if(event.Mouse.ButtonsPressed & 1)
                {
                    Window *dragWnd = nullptr;
                    Window *mWnd = nullptr;
                    for(Window *wnd : WM->windows)
                    {
                        Rectangle rect = wnd->ToRectangle();
                        Rectangle dragRect = wnd->DragRectangle;
                        dragRect.Origin += rect.Origin;
                        dragRect = rect.Intersection(dragRect);

                        if(rect.Contains(WM->mousePos) && wnd->ID != WM->mouseWndId)
                        {
                            mWnd = wnd;
                            dragWnd = nullptr;
                            if(dragRect.Contains(WM->mousePos) && wnd->ID != WM->mouseWndId)
                                dragWnd = wnd;
                        }
                    }
                    if(mWnd && mWnd->ID != WM->activeWindowId)
                    {
                        inhibitEvent = true;
                        WM->activeWindowId = mWnd->ID;
                        BringWindowToFront_nolock(mWnd->ID);
                    }
                    if(dragWnd && dragWnd->ID != WM->dragWindowId)
                        WM->dragWindowId = mWnd->ID;
                }

                if(event.Mouse.ButtonsReleased & 1)
                {
                    WM->drag = false;
                    WM->dragWindowId = -1;
                }

                if(d && event.Mouse.ButtonsHeld & 1)
                {
                    if(!WM->drag)
                    {
                        Point wPos;
                        GetWindowPosition_nolock(WM->activeWindowId, &wPos);
                        WM->dragPoint = WM->mousePos - wPos;
                    }
                    inhibitEvent = WM->dragWindowId >= 0;
                    WM->drag = true;
                    if(InputDevice::PeekEvent().DeviceType != InputDevice::Type::Mouse)
                    {
                        Point pos = WM->mousePos - WM->dragPoint;
                        SetWindowPosition_nolock(WM->dragWindowId, pos.X, pos.Y);
                    }
                }

                if(!inhibitEvent)
                {
                    Rectangle rect = GetWindowRectangle_nolock(WM->activeWindowId);
                    if(rect.Contains(WM->mousePos))
                        PutEvent_nolock(WM->activeWindowId, event);
                }

                WM->lock.Release();
            }
        }
        else if(event.DeviceType == InputDevice::Type::Keyboard)
            PutEvent(WM->activeWindowId, event);
    }
}

WindowManager::WindowManager(FrameBuffer *fb) :
    fb(fb), backBuffer(fb->Pixels, fb->Pixels->Format),
    lock("WM")
{
    Window *desktop = new Window(this, 0, 0, fb->Pixels->Width, fb->Pixels->Height, nullptr);
    desktopRect = desktop->ToRectangle();
    windows.Prepend(desktop);
    desktop->Contents->Clear(PixMap::Color::DarkGray);
    Window *debugWnd = new Window(this, 100, 200, 700, 400, nullptr);
    activeWindowId = debugWnd->ID;
    windows.Append(debugWnd);

    PixMap *cursorImage = PixMap::LoadCUR("WOOT_OS:/normal.cur", 0, &mouseHotspot.X, &mouseHotspot.Y);
    Window *mouseWnd = cursorImage ? new Window(this, 0, 0, cursorImage->Width, cursorImage->Height, &PixMap::PixelFormat::A8R8G8B8) : nullptr;
    mouseWndId = mouseWnd->ID;
    mouseWnd->UseAlpha = true;
    mouseWnd->Contents->Clear(PixMap::Color::Transparent);
    mouseWnd->Contents->AlphaBlit(cursorImage, 0, 0, 0, 0, cursorImage->Width, cursorImage->Height);
    delete cursorImage;
    windows.Append(mouseWnd);

    desktop->Visible = true;
    debugWnd->Visible = true;
    mouseWnd->Visible = true;

    inputThreadFinished = new Semaphore(0);
    thread = new Thread("input thread", Process::GetCurrent(), (void *)inputThread, (uintptr_t)this, DEFAULT_STACK_SIZE, 0, nullptr, inputThreadFinished, false);
    thread->Enable();
    thread->Resume(false);
}

void WindowManager::Initialize(FrameBuffer *fb)
{
    DefaultPixelFormat = fb->Pixels->Format;
    WM = new WindowManager(fb);
}

void WindowManager::DestroyThreadWindows_nolock(Thread *thread)
{
    for(;;)
    {
        bool anythingFound = false;
        for(Window *wnd : WM->windows)
        {
            if(wnd->Owner == thread)
            {
                DestroyWindow_nolock(wnd->ID);
                anythingFound = true;
                break;
            }
        }
        if(!anythingFound)
            break;
    }
}

void WindowManager::DestroyThreadWindows(Thread *thread)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return;
    DestroyThreadWindows_nolock(thread);
    WM->lock.Release();
}

WindowManager::Window *WindowManager::GetByID_nolock(int id)
{
    Window *res = nullptr;
    for(Window *wnd : WM->windows)
    {
        if(wnd->ID == id)
        {
            res = wnd;
            break;
        }
    }
    return res;
}

WindowManager::Window *WindowManager::GetByID(int id)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return nullptr;
    Window *wnd = GetByID_nolock(id);
    WM->lock.Release();
    return wnd;
}

int WindowManager::CreateWindow_nolock(int x, int y, int w, int h, PixMap::PixelFormat *fmt)
{
    if(!WM) return -EINVAL;
    Window *wnd = new Window(WM, x, y, w, h, fmt);
    Window *mouseWnd = GetByID_nolock(WM->mouseWndId);
    WM->windows.InsertBefore(wnd, mouseWnd, nullptr);
    int id = wnd->ID;
    WM->activeWindowId = id;
    return id;
}

int WindowManager::CreateWindow(int x, int y, int w, int h, PixMap::PixelFormat *fmt)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return -EBUSY;
    int id = CreateWindow_nolock(x, y, w, h, fmt);
    WM->lock.Release();
    return id;
}

bool WindowManager::DestroyWindow_nolock(int id)
{
    if(id <= 0 || !WM)
        return false;
    Window *wnd = GetByID_nolock(id);
    if(!wnd)
        return false;
    WM->windows.Remove(wnd, nullptr, false);
    Window *desktop = GetByID_nolock(0);
    if(desktop)
    {
        Rectangle wndRect = wnd->ToRectangle();
        desktop->Dirty.Add(wndRect);
        desktop->Update_nolock();
    }
    delete wnd;

    Window *prev = nullptr;
    for(Window *wnd : WM->windows)
    {
        if(prev && wnd->ID == WM->mouseWndId)
        {
            WM->activeWindowId = prev->ID;
            break;
        }
        prev = wnd;
    }

    return true;
}

bool WindowManager::DestroyWindow(int id)
{
    if(id <= 0 || !WM)
        return false;
    if(!WM->lock.Acquire(0, false))
        return false;
    int res = DestroyWindow_nolock(id);
    WM->lock.Release();
    return res;
}

bool WindowManager::ShowWindow_nolock(int id)
{
    if(id <= 0) return false;
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Visible = true;
    wnd->Invalidate_nolock();
    wnd->Update_nolock();
    return true;
}

bool WindowManager::ShowWindow(int id)
{
    if(id <= 0 || !WM)
        return false;
    if(!WM->lock.Acquire(0, false))
        return false;
    bool res = ShowWindow_nolock(id);
    WM->lock.Release();
    return res;
}

bool WindowManager::HideWindow_nolock(int id)
{
    if(id <= 0) return false;
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Visible = false;

    Window *desktop = GetByID_nolock(0);
    if(desktop)
    {
        Rectangle updateRect = wnd->ToRectangle();
        desktop->Dirty.Add(updateRect);
        desktop->Update();
    }
    return true;
}

bool WindowManager::HideWindow(int id)
{
    if(id <= 0 || !WM)
        return false;
    if(!WM->lock.Acquire(0, false))
        return false;
    bool res = HideWindow_nolock(id);
    WM->lock.Release();
    return res;
}

bool WindowManager::BringWindowToFront_nolock(int id)
{
    if(id <= 0 || !WM)
        return false;
    Window *wnd = GetByID_nolock(id);
    if(!wnd)
        return false;
    Window *mouseWnd = GetByID_nolock(WM->mouseWndId);
    WM->windows.Remove(wnd, nullptr, false);
    wnd->Visible = true;
    WM->windows.InsertBefore(wnd, mouseWnd, nullptr);
    wnd->Invalidate_nolock();
    wnd->Update_nolock();
    return true;
}

bool WindowManager::BringWindowToFront(int id)
{
    if(id <= 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    BringWindowToFront_nolock(id);
    WM->lock.Release();
    return true;
}

bool WindowManager::GetWindowPosition_nolock(int id, WindowManager::Point *pt)
{
    Window *wnd = GetByID_nolock(id);
    Rectangle rect = wnd->ToRectangle();
    if(pt) *pt = rect.Origin;
    if(!wnd) return false;
    return true;
}

bool WindowManager::GetWindowPosition(int id, WindowManager::Point *pt)
{
    if(id <= 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = GetWindowPosition_nolock(id, pt);
    WM->lock.Release();
    return true;
}

bool WindowManager::SetWindowPosition_nolock(int id, int x, int y)
{
    if(id <= 0) return false;
    Window *desktop = GetByID_nolock(0);
    if(!desktop) return false;
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    if(wnd->Visible)
    {
        Rectangle currRect = wnd->ToRectangle();
        wnd->Position.X = x;
        wnd->Position.Y = y;
        Rectangle newRect = wnd->ToRectangle();
        Rectangle updateRect = currRect;
        updateRect.Add(newRect);

        desktop->Dirty.Add(updateRect);
        desktop->Update_nolock();
    }
    return true;
}

bool WindowManager::SetWindowPosition(int id, int x, int y)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return false;
    SetWindowPosition_nolock(id, x, y);
    WM->lock.Release();
    return true;
}

bool WindowManager::DrawRectangle_nolock(int id, WindowManager::Rectangle rect, PixMap::Color color)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Contents->Rectangle(rect.Origin.X, rect.Origin.Y, rect.Width, rect.Height, color);
    wnd->Dirty.Add(rect);
    return true;
}

bool WindowManager::DrawRectangle(int id, Rectangle rect, PixMap::Color color)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = DrawRectangle_nolock(id, rect, color);
    WM->lock.Release();
    return res;
}

bool WindowManager::DrawFilledRectangle_nolock(int id, WindowManager::Rectangle rect, PixMap::Color color)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Contents->FillRectangle(rect.Origin.X, rect.Origin.Y, rect.Width, rect.Height, color);
    wnd->Dirty.Add(rect);
    return true;
}

bool WindowManager::DrawFilledRectangle(int id, Rectangle rect, PixMap::Color color)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = DrawFilledRectangle_nolock(id, rect, color);
    WM->lock.Release();
    return res;
}

bool WindowManager::UpdateWindow_nolock(int id)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Update_nolock();
    return true;
}

bool WindowManager::UpdateWindow(int id)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = UpdateWindow_nolock(id);
    WM->lock.Release();
    return res;
}

bool WindowManager::RedrawWindow_nolock(int id)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Invalidate_nolock();
    wnd->Update_nolock();
    return true;
}

bool WindowManager::RedrawWindow(int id)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = RedrawWindow_nolock(id);
    WM->lock.Release();
    return res;
}

bool WindowManager::DrawLine_nolock(int id, int x1, int y1, int x2, int y2, PixMap::Color color)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Contents->Line(x1, y1, x2, y2, color);
    if(x2 < x1) swap(int, x1, x2);
    if(y2 < y1) swap(int, y1, y2);
    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;
    wnd->Dirty.Add(Rectangle(x1, y1, w, h));
    return true;
}

bool WindowManager::DrawLine(int id, int x1, int y1, int x2, int y2, PixMap::Color color)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = DrawLine_nolock(id, x1, y1, x2, y2, color);
    WM->lock.Release();
    return res;
}

bool WindowManager::Blit_nolock(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Contents->Blit(src, sx, sy, x, y, w, h);
    wnd->Dirty.Add(Rectangle(x, y, w, h));
    return true;
}

bool WindowManager::Blit(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = Blit_nolock(id, src, sx, sy, x, y, w, h);
    WM->lock.Release();
    return res;
}

bool WindowManager::AlphaBlit_nolock(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Contents->AlphaBlit(src, sx, sy, x, y, w, h);
    wnd->Dirty.Add(Rectangle(x, y, w, h));
    return true;
}

bool WindowManager::AlphaBlit(int id, PixMap *src, int sx, int sy, int x, int y, int w, int h)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = AlphaBlit_nolock(id, src, sx, sy, x, y, w, h);
    WM->lock.Release();
    return res;
}

bool WindowManager::InvalidateRectangle_nolock(int id, WindowManager::Rectangle &rect)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->Dirty.Add(rect);
    return true;
}

bool WindowManager::InvalidateRectangle(int id, WindowManager::Rectangle &rect)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = InvalidateRectangle_nolock(id, rect);
    WM->lock.Release();
    return true;
}

void WindowManager::SetMousePosition_nolock(WindowManager::Point pos)
{
    WM->mousePos = pos;
    pos = pos - WM->mouseHotspot;
    SetWindowPosition_nolock(WM->mouseWndId, pos.X, pos.Y);
}

void WindowManager::SetMousePosition(WindowManager::Point pos)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return;
    SetMousePosition_nolock(pos);
    WM->lock.Release();
}

WindowManager::Point WindowManager::GetMousePosition_nolock()
{
    return WM ? WM->mousePos : Point();
}

WindowManager::Point WindowManager::GetMousePosition()
{
    if(!WM || !WM->lock.Acquire(0, false))
        return Point();
    Point res = GetMousePosition_nolock();
    WM->lock.Release();
    return res;
}

bool WindowManager::PutEvent_nolock(int id, InputDevice::Event event)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    return wnd->Events.Put(event, 0, true);
}

bool WindowManager::PutEvent(int id, InputDevice::Event event)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = PutEvent_nolock(id, event);
    WM->lock.Release();
    return res;
}

bool WindowManager::SetDragRectangle_nolock(int id, WindowManager::Rectangle rect)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return false;
    wnd->DragRectangle = rect;
    return true;
}

bool WindowManager::SetDragRectangle(int id, WindowManager::Rectangle rect)
{
    if(id < 0 || !WM || !WM->lock.Acquire(0, false))
        return false;
    bool res = SetDragRectangle_nolock(id, rect);
    WM->lock.Release();
    return res;
}

InputDevice::Event WindowManager::GetEvent_nolock(int id)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return InputDevice::Event();
    return wnd->Events.Get(0, nullptr);
}

InputDevice::Event WindowManager::GetEvent(int id)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return InputDevice::Event();
    InputDevice::Event res = GetEvent_nolock(id);
    WM->lock.Release();
    return res;
}

InputDevice::Event WindowManager::PeekEvent_nolock(int id, bool remove)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return InputDevice::Event();
    bool ok = false;
    InputDevice::Event res = wnd->Events.Peek(&ok);
    if(ok) wnd->Events.Get(0, nullptr);
    return res;
}

InputDevice::Event WindowManager::PeekEvent(int id, bool remove)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return InputDevice::Event();
    InputDevice::Event res = PeekEvent_nolock(id, remove);
    WM->lock.Release();
    return res;
}

WindowManager::Rectangle WindowManager::GetWindowRectangle_nolock(int id)
{
    Window *wnd = GetByID_nolock(id);
    if(!wnd) return Rectangle();
    return wnd->ToRectangle();
}

WindowManager::Rectangle WindowManager::GetWindowRectangle(int id)
{
    if(!WM || !WM->lock.Acquire(0, false))
        return Rectangle();
    Rectangle res = GetWindowRectangle_nolock(id);
    WM->lock.Release();
    return res;
}

void WindowManager::Cleanup()
{
    if(WM) delete WM;
}

WindowManager::Window::Window(WindowManager *wm, int x, int y, int w, int h, PixMap::PixelFormat *fmt) :
    ID(ids.GetNext()),
    Owner(Thread::GetCurrent()),
    Manager(wm),
    Position(x, y),
    Contents(new PixMap(w, h, fmt ? *fmt : DefaultPixelFormat)),
    Dirty(0, 0, w, h),
    Events(64),
    DragRectangle(0, 0, w, h)
{
}

WindowManager::Window::~Window()
{
    if(Contents) delete Contents;
}

void WindowManager::Window::Invalidate_nolock()
{
    if(!Manager)
        return;
    Rectangle d(0, 0, Contents->Width, Contents->Height);
    Dirty.Add(d);
}

void WindowManager::Window::Invalidate()
{
    if(!Manager || !Manager->lock.Acquire(0, false))
        return;
    Invalidate_nolock();
    Manager->lock.Release();
}

WindowManager::Rectangle WindowManager::Window::ToRectangle()
{
    return Rectangle(Position, Contents->Width, Contents->Height);
}

void WindowManager::Window::Update_nolock()
{
    if(!Manager)
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

        if(wnd->UseAlpha)
            Manager->backBuffer.AlphaBlit(wnd->Contents, localRect.Origin.X, localRect.Origin.Y, isect.Origin.X, isect.Origin.Y, isect.Width, isect.Height);
        else
            Manager->backBuffer.Blit(wnd->Contents, localRect.Origin.X, localRect.Origin.Y, isect.Origin.X, isect.Origin.Y, isect.Width, isect.Height);
    }

    Dirty = Rectangle(0, 0, 0, 0);
    Manager->fb->Pixels->Blit(&Manager->backBuffer,
                              updateRect.Origin.X, updateRect.Origin.Y,
                              updateRect.Origin.X, updateRect.Origin.Y,
                              updateRect.Width, updateRect.Height);
}

void WindowManager::Window::Update()
{
    if(!Manager || !Manager->lock.Acquire(0, false))
        return;
    Update_nolock();
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

WindowManager::Point::Point() :
    X(0), Y(0)
{
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

WindowManager::Point WindowManager::Point::operator +(WindowManager::Point p)
{
    return Point(X + p.X, Y + p.Y);
}

WindowManager::Point WindowManager::Point::operator -(WindowManager::Point p)
{
    return Point(X - p.X, Y - p.Y);
}

WindowManager::Point WindowManager::Point::operator +=(WindowManager::Point p)
{
    X += p.X;
    Y += p.Y;
    return *this;
}
