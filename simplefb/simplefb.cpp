#include <simplefb.h>

SimpleFB::SimpleFB(void *addr, int width, int height, int bpp, size_t pitch, int reds, int grns, int blus, int redb, int grnb, int blub)
{
    PixMap::PixelFormat format(bpp, 0, reds, grns, blus, 0, redb, grnb, blub);
    Pixels = new PixMap(width, height, pitch, format, addr, false);
}

int SimpleFB::GetModeCount()
{
    return 1;
}

int SimpleFB::GetModes(FrameBuffer::ModeInfo *buffer, size_t maxModes)
{
    if(maxModes < 1) return 0;
    buffer[0] = FrameBuffer::ModeInfo(Pixels->Width, Pixels->Height, Pixels->Pitch, Pixels->Format);
    return 1;
}

SimpleFB::~SimpleFB()
{
}
