#include <cpu.h>
#include <debugstream.h>
#include <errno.h>
#include <string.h>

#define EXTRA_RETURN
#define USE_VGA_TEXT

#ifdef USE_VGA_TEXT
static word *vgaText = (word *)0xC00B8000;
static uint vgaWidth = 80;
static uint vgaHeight = 25;
static uint vgaCurX = 0;
static uint vgaCurY = 0;
static word vgaAttrib = 0x0700;

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

DebugStream::DebugStream(word port)
    : port(port)
{
#ifdef USE_VGA_TEXT
    wmemset(vgaText, vgaAttrib, vgaWidth * vgaHeight);
    vgaSetCursorPos(0);
#endif // USE_VGA_TEXT
}

int64_t DebugStream::Read(void *buffer, int64_t n)
{
    return -ENOSYS;
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
    }
    cpuRestoreInterrupts(ints);
    return n;
}

DebugStream::~DebugStream()
{
}
