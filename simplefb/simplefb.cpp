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

SimpleFB::~SimpleFB()
{

}
