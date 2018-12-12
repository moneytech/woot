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

    struct uiLineEdit *display = uiLineEditCreate(wnd->RootControl, 4, 4, wnd->ClientRectangle.Width - 8, 24, "0", NULL, NULL);
    struct uiButton *buttons[20];
    char *btnLabels[20] =
    {
        "CE", "sqrt", "%", "1/x",
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", ".", "=", "+"
    };
    int bw = wnd->ClientRectangle.Width / 4;
    int bh = (wnd->ClientRectangle.Height - 32) / 5;
    for(int y = 0; y < 5; ++y)
    {
        for(int x = 0; x < 4; ++x)
        {
            int btn = x + (4 * y);
            buttons[btn] = uiButtonCreate(wnd->RootControl, 4 + x * bw, 4 + 32 + y * bh, bw - 8, bh - 8, btnLabels[btn], NULL, NULL);
            uiControlSetOnMousePress((struct uiControl *)buttons[btn], mousePressTest);
        }
    }

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
