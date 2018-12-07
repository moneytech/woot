#include <woot/font.h>
#include <woot/pixmap.h>
#include <woot/vkeys.h>
#include <woot/wm.h>

int main(int argc, char *argv[])
{
    wmInitialize();
    struct wmWindow *wnd = wmCreateWindow(400, 300, 200, 300, "Calculator", 1);
    struct pmPixMap *pm = wnd->ClientArea;
    wmShowWindow(wnd);

    for(;;)
    {
        struct wmEvent event;
        wmGetEvent(wnd->ID, &event);
        wmProcessEvent(wnd, &event);
        if(event.Type == ET_KEYBOARD)
        {
            if(event.Keyboard.Key == VK_ESCAPE)
                break;
        }
    }

    wmDestroyWindow(wnd->ID);
    wmCleanup();
    return 0;
}
