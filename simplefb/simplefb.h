#ifndef SIMPLEFB_H
#define SIMPLEFB_H

#include <framebuffer.h>

class SimpleFB : public FrameBuffer
{
    void *addr;
public:
    SimpleFB(void *addr);
    virtual int GetWidth();
    virtual int GetHeight();
    virtual ~SimpleFB();
};

#endif // SIMPLEFB_H
