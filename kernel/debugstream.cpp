#include <cpu.h>
#include <debugstream.h>
#include <errno.h>
#include <fbfont.h>
#include <framebuffer.h>
#include <inputdevice.h>
#include <string.h>

#define EXTRA_RETURN
//#define USE_VGA_TEXT
#define USE_SERIAL

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

#ifdef USE_SERIAL
#define SERIAL_PORT 0x03F8
static bool serialHasData()
{
    return _inb(SERIAL_PORT + 5) & 0x01;
}

static char serialGetChar()
{
    return _inb(SERIAL_PORT);
}

static char serialRead()
{
    while(!serialHasData());
    return serialGetChar();
}

static void serialWrite(char c)
{
    while(!(_inb(SERIAL_PORT + 5) & 0x20));
    _outb(SERIAL_PORT, c);
}
#endif // USE_SERIAL

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

#ifdef USE_SERIAL
    // initialize some kind of serial port communication
    // so we can debug on real hardware easier
    _outb(SERIAL_PORT + 1, 0x00);    // No interrupts
    _outb(SERIAL_PORT + 3, 0x80);    // Enable DLAB
    _outb(SERIAL_PORT + 0, 0x01);    // 115200 baud rate
    _outb(SERIAL_PORT + 1, 0x00);
    _outb(SERIAL_PORT + 3, 0x03);    // Disable DLAB and set 8N1
    _outb(SERIAL_PORT + 2, 0xC7);    // FIFO 14 bytes
    _outb(SERIAL_PORT + 4, 0x03);    // RTS and DTR asserted
#endif // USE_SERIAL
}

void DebugStream::SetWindow(WindowManager::Window *pixmap)
{
    this->window = pixmap;
    winW = pixmap->Contents->Width / FONT_BITS;
    winH = pixmap->Contents->Height / FONT_SCANLINES;
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
    static int mx = 0;
    static int my = 0;
    static int wid = 2;

    if(!n) return 0;
    if(!buffer) return -EINVAL;

    for(;;)
    {
        char chr = 0;
        if(window)
        {
            bool ok = false;
            InputDevice::Event event = window->Events.Get(0, &ok);
            if(ok && event.DeviceType == InputDevice::Type::Keyboard)
            {
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
                chr = vkToChar(event.Keyboard.Key, shift, caps, num);
            }
#ifdef USE_SERIAL
            /*else if(serialHasData())
            {
                WriteStr("-");
                chr = serialGetChar();
                if(chr == '\r') chr = '\n';
            }*/
#endif // USE_SERIAL
        }
        else
        {
#ifdef USE_SERIAL
            chr = serialRead();
            if(chr == '\r') chr = '\n';
#else // USE_SERIAL
            cpuSystemHalt(0x1234ABCD);
#endif // USE_SERIAL
        }
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
        {
            _outb(port, '\r');
#ifdef USE_SERIAL
            serialWrite('\r');
#endif // USE_SERIAL
        }
#endif // EXTRA_RETURN
        _outb(port, c);
#ifdef USE_SERIAL
            serialWrite(c);
#endif // USE_SERIAL
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
        if(window)
        {
            bool back = c == '\b';
            if(back)
            {
                c = ' ';
                if(fbX) --fbX;
                else
                {
                    fbX = winW - 1;
                    --fbY;
                }
            }

            if(c == '\n')
                ++fbY, fbX = 0;
            else if(c == '\r')
                fbX = 0;
            else
            {
                byte *glyph = fbFont[c];
                PixMap::Color bg = window->Contents->GetPixel(window->Contents->Width - 1, window->Contents->Height - 1);

                auto drawGlyph = [this, glyph](WindowManager::Window *window, int ox, int oy, PixMap::Color c, PixMap::Color bc)
                {
                    for(int y = 0; y < FONT_SCANLINES; ++y)
                    {
                        int glyphLine = glyph[y];
                        for(int x = 0; x < FONT_BITS; ++x)
                            window->Contents->SetPixel(x + fbX * FONT_BITS + ox, y + fbY * FONT_SCANLINES + oy, glyphLine & (0x80 >> x) ? c : bc);
                    }
                    window->Dirty.Add(WindowManager::Rectangle(fbX * FONT_BITS, fbY * FONT_SCANLINES, FONT_BITS, FONT_SCANLINES));
                    window->Update();
                };
                drawGlyph(window, 0, 0, PixMap::Color(255, 255, 255), bg);

                if(!back) ++fbX;
            }

            if(fbX >= winW)
            {
                fbX = 0;
                ++fbY;
            }
            if(fbY >= winH)
            {
                window->Contents->Blit(window->Contents, 0, FONT_SCANLINES, 0, 0, window->Contents->Width, window->Contents->Height - FONT_SCANLINES);
                PixMap::Color c(window->Contents->GetPixel(window->Contents->Width - 1, window->Contents->Height - 1));
                window->Contents->FillRectangle(0, window->Contents->Height - FONT_SCANLINES, window->Contents->Width, FONT_SCANLINES, c);
                window->Invalidate();
                window->Update();
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
