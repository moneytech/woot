#ifndef KWM_H
#define KWM_H

#include <types.h>

class FrameBuffer;

class WindowManager
{
    static WindowManager *wm;

    FrameBuffer *fb;

    WindowManager(FrameBuffer *fb);
public:
    static void Initialize(FrameBuffer *fb);
    static void Cleanup();
};

#endif // KWM_H
