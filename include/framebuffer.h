#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <list.h>
#include <mutex.h>
#include <pixmap.h>
#include <sequencer.h>
#include <types.h>

class FrameBuffer
{
    static Sequencer<int> ids;
    static List<FrameBuffer *> fbs;
    static Mutex listLock;
    Mutex lock;
public:
    struct ModeInfo
    {
        int Width, Height;
        size_t Pitch;
        PixMap::PixelFormat Format;

        ModeInfo();
        ModeInfo(int width, int height, size_t pitch, PixMap::PixelFormat format);
    };
protected:
    int ID;
public:
    PixMap *Pixels;

    static int Add(FrameBuffer *fb);
    static FrameBuffer *GetByID(int id, bool lock);
    static int Remove(FrameBuffer *fb);

    FrameBuffer();
    int Lock();
    void *GetPixels();
    void UnLock();

    virtual int GetModeCount();
    virtual int GetModes(ModeInfo *buffer, size_t maxModes);
    virtual int SetMode(ModeInfo mode);
    virtual ~FrameBuffer();
};

#endif // FRAMEBUFFER_H
