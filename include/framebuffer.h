#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <types.h>

class FrameBuffer
{
public:
    FrameBuffer();
    virtual int GetWidth();
    virtual int GetHeight();
    virtual ~FrameBuffer();
};

#endif // FRAMEBUFFER_H
