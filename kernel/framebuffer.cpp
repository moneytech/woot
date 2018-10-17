#include <errno.h>
#include <framebuffer.h>
#include <mutex.h>
#include <stdlib.h>
#include <string.h>

Sequencer<int> FrameBuffer::ids(0);
List<FrameBuffer *> FrameBuffer::fbs;
Mutex FrameBuffer::listLock;

int FrameBuffer::Add(FrameBuffer *fb)
{
    if(!listLock.Acquire(5000, false))
        return -EBUSY;
    fbs.Append(fb);
    fb->ID = ids.GetNext();
    listLock.Release();
    return 0;
}

FrameBuffer *FrameBuffer::GetByID(int id, bool lock)
{
    if(!listLock.Acquire(5000, false))
        return nullptr;
    for(FrameBuffer *fb : fbs)
    {
        if(fb->ID == id)
        {
            if(lock && fb->Lock())
                return nullptr;
            return fb;
        }
    }
    listLock.Release();
    return nullptr;
}

int FrameBuffer::Remove(FrameBuffer *fb)
{
    if(!listLock.Acquire(5000, false))
        return -EBUSY;
    int res = fbs.Remove(fb, nullptr, false);
    listLock.Release();
    return res;
}

FrameBuffer::FrameBuffer() :
    ID(-1)
{
}

FrameBuffer::ModeInfo FrameBuffer::GetMode()
{
    return Mode;
}

int FrameBuffer::Lock()
{
    return lock.Acquire(5000, false) ? 0 : -EBUSY;
}

void *FrameBuffer::GetPixels()
{
    return Pixels;
}

void FrameBuffer::UnLock()
{
    return lock.Release();
}

int FrameBuffer::GetModeCount()
{
    return -ENOSYS;
}

int FrameBuffer::GetModes(FrameBuffer::ModeInfo *buffer, size_t maxModes)
{
    return -ENOSYS;
}

int FrameBuffer::SetMode(FrameBuffer::ModeInfo mode)
{
    return -ENOSYS;
}

void FrameBuffer::SetPixel(int x, int y, FrameBuffer::Color c)
{
    if(x < 0 || x >= Mode.Width || y < 0 || y >= Mode.Height)
        return;
    uint32_t col = c.ToValue(Mode);
    if(Mode.BPP == 32)
    {
        uint32_t *pixel = (uint32_t *)(PixelBytes + Mode.Pitch * y + x * 4);
        *pixel = col;
    }
    else if(Mode.BPP == 24)
    {
        uint32_t *pixel = (uint32_t *)(PixelBytes + Mode.Pitch * y + x * 3);
        *pixel = (*pixel & 0xFF000000) | col;
    }
    else if(Mode.BPP == 16 || Mode.BPP == 15)
    {
        uint16_t *pixel = (uint16_t *)(PixelBytes + Mode.Pitch * y + x * 2);
        *pixel = col;
    }
}

FrameBuffer::Color FrameBuffer::GetPixel(int x, int y)
{
    if(x < 0 || x >= Mode.Width || y < 0 || y >= Mode.Height)
        return Color();
    if(Mode.BPP == 32)
    {
        uint32_t *pixel = (uint32_t *)(PixelBytes + Mode.Pitch * y + x * 4);
        return Color::FromValue(Mode, *pixel);
    }
    else if(Mode.BPP == 24)
    {
        uint32_t *pixel = (uint32_t *)(PixelBytes + Mode.Pitch * y + x * 3);
        return Color::FromValue(Mode, *pixel & 0xFFFFFF);
    }
    else if(Mode.BPP == 16 || Mode.BPP == 15)
    {
        uint16_t *pixel = (uint16_t *)(PixelBytes + Mode.Pitch * y + x * 2);
        return Color::FromValue(Mode, *pixel);
    }
    return Color();
}

void FrameBuffer::Clear(FrameBuffer::Color c)
{
    FillRectangle(0, 0, Mode.Width, Mode.Height, c);
}

void FrameBuffer::FillRectangle(int x, int y, int w, int h, FrameBuffer::Color c)
{
    if(w <= 0 || h <= 0)
        return;

    int x2 = x + w;
    int y2 = y + h;

    if(x2 < 0 || x >= Mode.Width || y2 < 0 || y >= Mode.Height)
        return;

    x = clamp(x, 0, Mode.Width);
    y = clamp(y, 0, Mode.Height);
    x2 = clamp(x2, 0, Mode.Width);
    y2 = clamp(y2, 0, Mode.Height);
    w = x2 - x;
    h = y2 - y;

    uint32_t col = c.ToValue(Mode);

    if(Mode.BPP == 32)
    {
        for(int Y = y; Y < x2; ++Y)
        {
            uint32_t *line = (uint32_t *)(PixelBytes + Y * Mode.Pitch);
            lmemset(line + x, col, w);
        }
    }
    else if(Mode.BPP == 24)
    {
        for(int Y = y; Y < x2; ++Y)
        {
            for(int X = x; X < x2; ++X)
            {
                uint32_t *pixel = (uint32_t *)(PixelBytes + Mode.Pitch * Y + X * 3);
                *pixel = (*pixel & 0xFF000000) | col;
            }
        }
    }
    else if(Mode.BPP == 16 || Mode.BPP == 15)
    {
        for(int Y = y; Y < x2; ++Y)
        {
            uint16_t *line = (uint16_t *)(PixelBytes + Y * Mode.Pitch);
            wmemset(line + x, col, w);
        }
    }
}

FrameBuffer::~FrameBuffer()
{
    Lock();
}

FrameBuffer::Color::Color()
{
}

FrameBuffer::Color::Color(const FrameBuffer::Color &src) :
    Value(src.Value)
{
}

FrameBuffer::Color::Color(uint8_t r, uint8_t g, uint8_t b) :
    R(r), G(g), B(b)
{
}

FrameBuffer::Color::Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b) :
    A(a), R(r), G(g), B(b)
{
}


FrameBuffer::Color::Color(uint32_t value) :
    Value(value)
{
}

FrameBuffer::Color FrameBuffer::Color::FromFloatRGB(float r, float g, float b)
{
    return Color(clamp((int)(r * 255), 0, 255),
                 clamp((int)(g * 255), 0, 255),
                 clamp((int)(b * 255), 0, 255));
}

uint32_t FrameBuffer::Color::ToValue(ModeInfo &mode)
{
    uint32_t a = A >> (8 - mode.AlphaBits);
    uint32_t r = R >> (8 - mode.RedBits);
    uint32_t g = G >> (8 - mode.GreenBits);
    uint32_t b = B >> (8 - mode.BlueBits);
    return a << mode.AlphaShift | r << mode.RedShift | g << mode.GreenShift | b << mode.BlueShift;
}

FrameBuffer::Color FrameBuffer::Color::FromValue(FrameBuffer::ModeInfo &mode, uint32_t value)
{
    uint32_t A = ((value >> mode.AlphaShift) << (8 - mode.AlphaBits)) & 0xFF;
    uint32_t R = ((value >> mode.RedShift) << (8 - mode.RedBits)) & 0xFF;
    uint32_t G = ((value >> mode.GreenShift) << (8 - mode.GreenBits)) & 0xFF;
    uint32_t B = ((value >> mode.BlueShift) << (8 - mode.BlueBits)) & 0xFF;
    return Color(A, R, G, B);
}
