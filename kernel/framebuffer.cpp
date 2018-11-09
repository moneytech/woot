#include <errno.h>
#include <framebuffer.h>
#include <mutex.h>
#include <stdlib.h>
#include <string.h>

Sequencer<int> FrameBuffer::ids(0);
List<FrameBuffer *> FrameBuffer::fbs;
Mutex FrameBuffer::listLock;

int FrameBuffer::Add(FrameBuffer *fb)
{
    if(!listLock.Acquire(5000, false))
        return -EBUSY;
    fbs.Append(fb);
    fb->ID = ids.GetNext();
    listLock.Release();
    return 0;
}

FrameBuffer *FrameBuffer::GetByID(int id, bool lock)
{
    if(!listLock.Acquire(5000, false))
        return nullptr;
    for(FrameBuffer *fb : fbs)
    {
        if(fb->ID == id)
        {
            if(lock && fb->Lock())
                return nullptr;
            return fb;
        }
    }
    listLock.Release();
    return nullptr;
}

int FrameBuffer::Remove(FrameBuffer *fb)
{
    if(!listLock.Acquire(5000, false))
        return -EBUSY;
    int res = fbs.Remove(fb, nullptr, false);
    listLock.Release();
    return res;
}

FrameBuffer::FrameBuffer() :
    ID(-1)
{
}

int FrameBuffer::Lock()
{
    return lock.Acquire(5000, false) ? 0 : -EBUSY;
}

void *FrameBuffer::GetPixels()
{
    return Pixels->Pixels;
}

void FrameBuffer::UnLock()
{
    return lock.Release();
}

int FrameBuffer::GetModeCount()
{
    return -ENOSYS;
}

int FrameBuffer::GetModes(FrameBuffer::ModeInfo *buffer, size_t maxModes)
{
    return -ENOSYS;
}

int FrameBuffer::SetMode(FrameBuffer::ModeInfo mode)
{
    return -ENOSYS;
}

FrameBuffer::~FrameBuffer()
{
    Lock();
}

FrameBuffer::ModeInfo::ModeInfo()
{
    memset(this, 0, sizeof(ModeInfo));
}

FrameBuffer::ModeInfo::ModeInfo(int width, int height, size_t pitch, PixMap::PixelFormat format) :
    Width(width), Height(height), Pitch(pitch), Format(format)
{
}
