#include <cpu.h>
#include <debugstream.h>
#include <errno.h>
#include <fbfont.h>
#include <framebuffer.h>
#include <inputdevice.h>
#include <string.h>

#define EXTRA_RETURN
//#define USE_VGA_TEXT

#ifdef USE_VGA_TEXT
static word *vgaText = (word *)0xC00B8000;
static uint vgaWidth = 80;
static uint vgaHeight = 25;
static uint vgaCurX = 0;
static uint vgaCurY = 0;
static word vgaAttrib = 0x0700;

static void vgaSetCursorSize(byte start, byte end)
{
    _outb(0x3D4, 0x0A);
    _outb(0x3D5, (_inb(0x3D5) & 0xC0) | start);
    _outb(0x3D4, 0x0B);
    _outb(0x3D5, (_inb(0x3D5) & 0xE0) | end);
}

static void vgaSetCursorPos(uint16_t pos)
{
    bool cs = cpuDisableInterrupts();
    _outb(0x3D4, 0x0F);
    _outb(0x3D5, pos & 0xFF);
    _outb(0x3D4, 0x0E);
    _outb(0x3D5, pos >> 8);
    cpuRestoreInterrupts(cs);
}
#endif // USE_VGA_TEXT

static char vkToChar(VirtualKey vk, bool shift, bool caps, bool num)
{
    static const char *digits = "0123456789";
    static const char *shiftDigits = ")!@#$%^&*(";
    static const char *lowerLetters = "abcdefghijklmnopqrstuvwxyz";
    static const char *upperLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    unsigned int k = (unsigned int)vk;

    if(k >= (unsigned int)VirtualKey::NumPad0 && k <= (unsigned int)VirtualKey::NumPad9)
    {
        if(num)
            return digits[k - (unsigned int)VirtualKey::NumPad0];
    }
    else if(k >= (unsigned int)VirtualKey::Key0 && k <= (unsigned int)VirtualKey::Key9)
    {
        unsigned int dig = k - (unsigned int)VirtualKey::Key0;
        return shift ? shiftDigits[dig] : digits[dig];
    }
    else if(k >= (unsigned int)VirtualKey::KeyA && k <= (unsigned int)VirtualKey::KeyZ)
    {
        unsigned int let = k - (unsigned int)VirtualKey::KeyA;
        return ((shift != caps) ? upperLetters[let] : lowerLetters[let]);
    }
    else if(vk == VirtualKey::Space)
        return ' ';
    else if(vk == VirtualKey::Return)
        return '\n';
    else if(vk == VirtualKey::OEMMinus)
        return (shift ? '_' : '-');
    else if(vk == VirtualKey::OEMPlus)
        return (shift ? '+' : '=');
    else if(vk == VirtualKey::OEMComma)
        return (shift ? '<' : ',');
    else if(vk == VirtualKey::OEMPeriod)
        return (shift ? '>' : '.');
    else if(vk == VirtualKey::OEM1)
        return (shift ? ':' : ';');
    else if(vk == VirtualKey::OEM2)
        return (shift ? '?' : '/');
    else if(vk == VirtualKey::OEM3)
        return (shift ? '~' : '`');
    else if(vk == VirtualKey::OEM4)
        return (shift ? '{' : '[');
    else if(vk == VirtualKey::OEM5)
        return (shift ? '|' : '\\');
    else if(vk == VirtualKey::OEM6)
        return (shift ? '}' : ']');
    else if(vk == VirtualKey::OEM7)
        return (shift ? '"' : '\'');
    else if(vk == VirtualKey::Subtract)
        return '-';
    else if(vk == VirtualKey::Add)
        return '+';
    else if(vk == VirtualKey::Multiply)
        return '*';
    else if(vk == VirtualKey::Divide)
        return '/';
    else if(vk == VirtualKey::Decimal)
        return '.';
    else if(vk == VirtualKey::Back)
        return '\b';
    else if(vk == VirtualKey::Escape)
        return 0x1B;
    return 0;
}

DebugStream::DebugStream(word port)
    : port(port)
{
#ifdef USE_VGA_TEXT
    wmemset(vgaText, vgaAttrib, vgaWidth * vgaHeight);
    vgaSetCursorSize(13, 14);
    vgaSetCursorPos(0);
#endif // USE_VGA_TEXT
}

void DebugStream::SetFrameBuffer(FrameBuffer *fb)
{
    this->fb = fb;
    fbW = fb->Pixels->Width / FONT_BITS;
    fbH = fb->Pixels->Height / FONT_SCANLINES;
}

void DebugStream::EnableLineBuffer()
{
    lineBufferState = true;
    lineBufferPos = 0;
}

void DebugStream::DisableLineBuffer()
{
    lineBufferState = false;
}

int64_t DebugStream::Read(void *buffer, int64_t n)
{
    static bool shift = false, alt = false, ctrl = false, caps = false, num = false;

    if(!n) return 0;
    if(!buffer) return -EINVAL;

    for(;;)
    {
        InputDevice::Event event = InputDevice::GetEvent(0);
        if(event.DeviceType != InputDevice::Type::Keyboard)
            continue; // ignore non keyboard events
        if(event.Keyboard.Key == VirtualKey::LShift ||
                event.Keyboard.Key == VirtualKey::RShift)
        {
            shift = !event.Keyboard.Release;
            continue;
        }
        if(event.Keyboard.Key == VirtualKey::LMenu ||
                event.Keyboard.Key == VirtualKey::RMenu)
        {
            alt = !event.Keyboard.Release;
            continue;
        }
        if(event.Keyboard.Key == VirtualKey::LControl ||
                event.Keyboard.Key == VirtualKey::RControl)
        {
            ctrl = !event.Keyboard.Release;
            continue;
        }

        if(event.Keyboard.Release)
        {
            if(event.Keyboard.Key == VirtualKey::Capital)
                caps = !caps;
            if(event.Keyboard.Key == VirtualKey::NumLock)
                num = !num;
            continue;
        }
        if(ctrl && alt && event.Keyboard.Key == VirtualKey::Delete)
        {
            _outb(0x64, 0xFE);
            continue;
        }
        char chr = vkToChar(event.Keyboard.Key, shift, caps, num);
        if(!chr) continue;
        if(!lineBufferState)
        {
            *((byte *)buffer) = chr;
            return 1;
        }

        if(chr == '\b')
        {
            if(lineBufferPos)
            {
                Write("\b", 1);
                lineBuffer[--lineBufferPos] = 0;
            }
            continue;
        }
        if(lineBufferPos >= sizeof(lineBuffer))
            continue;
        Write(&chr, 1);
        if(chr == '\n')
        {
            lineBuffer[lineBufferPos] = '\n';
            break;
        }
        lineBuffer[lineBufferPos++] = chr;
        if(lineBufferPos >= n)
            break;
    }
    int res = lineBufferPos;
    memcpy(buffer, lineBuffer, lineBufferPos);
    lineBufferPos = 0;
    return res;
}

int64_t DebugStream::Write(const void *buffer, int64_t n)
{
    bool ints = cpuDisableInterrupts();
    byte *b = (byte *)buffer;
    for(int64_t i = 0; i < n; ++i)
    {
        byte c = *b++;
#ifdef EXTRA_RETURN
        if(c == '\n')
            _outb(port, '\r');
#endif // EXTRA_RETURN
        _outb(port, c);
#ifdef USE_VGA_TEXT
        if(c == '\n')
            ++vgaCurY, vgaCurX = 0;
        else if(c == '\b')
        {
            if(vgaCurX)
                --vgaCurX;
            vgaText[vgaCurY * vgaWidth + vgaCurX] = ' ' | vgaAttrib;
        }
        else if(c == '\r')
            vgaCurX = 0;
        else
        {
            vgaText[vgaCurY * vgaWidth + vgaCurX] = c | vgaAttrib;
            ++vgaCurX;
        }

        if(vgaCurX >= vgaWidth)
        {
            vgaCurX = 0;
            ++vgaCurY;
        }
        if(vgaCurY >= vgaHeight)
        {
            memcpy(vgaText, vgaText + vgaWidth, vgaWidth * (vgaHeight - 1) * sizeof(*vgaText));
            wmemset(vgaText + vgaWidth * (vgaHeight - 1), vgaAttrib, vgaWidth);
            vgaCurY = vgaHeight - 1;
        }
        vgaSetCursorPos(vgaCurY * vgaWidth + vgaCurX);
#endif // USE_VGA_TEXT
        if(fb)
        {
            bool back = c == '\b';
            if(back)
            {
                c = ' ';
                if(fbX) --fbX;
                else
                {
                    fbX = fbW - 1;
                    --fbY;
                }
            }

            if(c == '\n')
                ++fbY, fbX = 0;
            else if(!fb->Lock())
            {
                byte *glyph = fbFont[c];
                PixMap::Color bg = fb->Pixels->GetPixel(fb->Pixels->Width - 1, fb->Pixels->Height - 1);

                auto drawGlyph = [this, glyph](FrameBuffer *fb, int ox, int oy, PixMap::Color c, PixMap::Color bc)
                {
                    for(int y = 0; y < FONT_SCANLINES; ++y)
                    {
                        int glyphLine = glyph[y];
                        for(int x = 0; x < FONT_BITS; ++x)
                            fb->Pixels->SetPixel(x + fbX * FONT_BITS + ox, y + fbY * FONT_SCANLINES + oy, glyphLine & (0x80 >> x) ? c : bc);
                    }
                };
                drawGlyph(fb, 0, 0, PixMap::Color(255, 255, 255), bg);

                if(!back) ++fbX;
                fb->UnLock();
            }

            if(fbX >= fbW)
            {
                fbX = 0;
                ++fbY;
            }
            if(fbY >= fbH)
            {
                if(!fb->Lock())
                {
                    // TODO: Change me to blit transfer
                    byte *pixels = (byte *)fb->GetPixels();
                    memmove(pixels, pixels + FONT_SCANLINES * fb->Pixels->Pitch, (fb->Pixels->Height - FONT_SCANLINES) * fb->Pixels->Pitch);
                    PixMap::Color c(48, 64, 16);
                    fb->Pixels->FillRectangle(0, fb->Pixels->Height - FONT_SCANLINES, fb->Pixels->Width, FONT_SCANLINES, c);
                    fb->UnLock();
                }
                --fbY;
            }
        }
    }
    cpuRestoreInterrupts(ints);
    return n;
}

DebugStream::~DebugStream()
{
}
