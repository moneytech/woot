#include <simplefb.h>

SimpleFB::SimpleFB(void *addr, int width, int height, int bpp, size_t pitch, int reds, int grns, int blus, int redb, int grnb, int blub)
{
    Pixels = addr;
    Mode.Width = width;
    Mode.Height = height;
    Mode.BPP = bpp;
    Mode.Pitch = pitch;
    Mode.RedShift = reds;
    Mode.GreenShift = grns;
    Mode.BlueShift = blus;
    Mode.RedBits = redb;
    Mode.GreenBits = grnb;
    Mode.BlueBits = blub;
}

int SimpleFB::GetModeCount()
{
    return 1;
}

int SimpleFB::GetModes(FrameBuffer::ModeInfo *buffer, size_t maxModes)
{
    if(maxModes < 1) return 0;
    buffer[0] = Mode;
    return 1;
}

SimpleFB::~SimpleFB()
{
}
