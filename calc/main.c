#include <stdio.h>
#include <stdlib.h>
#include <woot/font.h>
#include <woot/pixmap.h>
#include <woot/ui.h>
#include <woot/vkeys.h>
#include <woot/wm.h>

void mouseMoveTest(struct uiControl *sender, struct wmEvent *event)
{
    printf("mouse move: %s %d %d %d %d %d\n", uiControlGetText(sender), event->Mouse.X, event->Mouse.Y, event->Mouse.ButtonsPressed, event->Mouse.ButtonsReleased, event->Mouse.ButtonsHeld);
}

void mousePressTest(struct uiControl *sender, struct wmEvent *event)
{
    printf("mouse press: %s %d %d %d %d %d\n", uiControlGetText(sender), event->Mouse.X, event->Mouse.Y, event->Mouse.ButtonsPressed, event->Mouse.ButtonsReleased, event->Mouse.ButtonsHeld);
}

void mouseReleaseTest(struct uiControl *sender, struct wmEvent *event)
{
    printf("mouse release: %s %d %d %d %d %d\n", uiControlGetText(sender), event->Mouse.X, event->Mouse.Y, event->Mouse.ButtonsPressed, event->Mouse.ButtonsReleased, event->Mouse.ButtonsHeld);
}

int main(int argc, char *argv[])
{
    wmInitialize();
    struct wmWindow *wnd = wmCreateWindow(400, 300, 200, 300, "Calculator", 1);
    struct pmPixMap *pm = wnd->ClientArea;
    wmShowWindow(wnd);

    struct uiButton *testButton = uiButtonCreate(wnd->RootControl, 50, 20, 50, 40, "Btn1", NULL, NULL);
    uiControlSetOnMouseMove((struct uiControl *)testButton, mouseMoveTest);
    uiControlSetOnMousePress((struct uiControl *)testButton, mousePressTest);
    uiControlSetOnMouseRelease((struct uiControl *)testButton, mouseReleaseTest);

    struct uiButton *testButton2 = uiButtonCreate(wnd->RootControl, 50, 70, 50, 40, "Btn2", NULL, NULL);
    uiControlSetOnMouseMove((struct uiControl *)testButton2, mouseMoveTest);
    uiControlSetOnMousePress((struct uiControl *)testButton2, mousePressTest);
    uiControlSetOnMouseRelease((struct uiControl *)testButton2, mouseReleaseTest);

    uiControlRedraw(wnd->RootControl);
    wmUpdateWindow(wnd);

    for(;;)
    {
        struct wmEvent event;
        wmGetEvent(wnd->ID, &event);
        wmProcessEvent(wnd, &event);
        if(event.Type == ET_KEYBOARD)
        {
            if(event.Keyboard.Key == VK_ESCAPE)
                break;
            else if(event.Keyboard.Key == VK_F12)
                wmRedrawWindow(wnd);
        }
    }

    wmDestroyWindow(wnd->ID);
    wmCleanup();
    return 0;
}
