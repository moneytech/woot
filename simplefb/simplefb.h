#ifndef SIMPLEFB_H
#define SIMPLEFB_H

#include <framebuffer.h>

class SimpleFB : public FrameBuffer
{
public:
    SimpleFB(void *addr, int width, int height, int bpp, size_t pitch, int reds, int grns, int blus, int redb, int grnb, int blub);
    virtual ~SimpleFB();
};

#endif // SIMPLEFB_H
