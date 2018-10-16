#include <cpu.h>
#include <debugstream.h>
#include <errno.h>
#include <fbfont.h>
#include <framebuffer.h>
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
    FrameBuffer::ModeInfo mode = fb->GetMode();
    fbW = mode.Width / FONT_BITS;
    fbH = mode.Height / FONT_SCANLINES;
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
            }

            if(c == '\n')
                ++fbY, fbX = 0;
            else if(!fb->Lock())
            {
                byte *glyph = fbFont[c];
                for(int y = 0; y < FONT_SCANLINES; ++y)
                {
                    int glyphLine = glyph[y];
                    for(int x = 0; x < FONT_BITS; ++x)
                    {
                        FrameBuffer::Color c(48, 64, 16);
                        if(glyphLine & (0x80 >> x)) c.R = 255;
                        fb->SetPixel(x + fbX * FONT_BITS, y + fbY * FONT_SCANLINES, c);
                    }
                }
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
                    byte *pixels = (byte *)fb->GetPixels();
                    FrameBuffer::ModeInfo mode = fb->GetMode();
                    memmove(pixels, pixels + FONT_SCANLINES * mode.Pitch, (mode.Height - FONT_SCANLINES) * mode.Pitch);
                    FrameBuffer::Color c(48, 64, 16);
                    fb->FillRectangle(0, mode.Height - FONT_SCANLINES, mode.Width, FONT_SCANLINES, c);
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
