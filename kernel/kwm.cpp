#include <framebuffer.h>
#include <kwm.h>

WindowManager *WindowManager::wm;

WindowManager::WindowManager(FrameBuffer *fb) :
    fb(fb)
{

}

void WindowManager::Initialize(FrameBuffer *fb)
{
    wm = new WindowManager(fb);
}

void WindowManager::Cleanup()
{
    if(wm) delete wm;
}
