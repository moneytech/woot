#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <woot/font.h>
#include <woot/pixmap.h>
#include <woot/ui.h>
#include <woot/vkeys.h>
#include <woot/wm.h>

long long pow10Table[] =
{
    1ll,
    10ll,
    100ll,
    1000ll,
    10000ll,
    100000ll,
    1000000ll,
    10000000ll,
    100000000ll,
    1000000000ll,
    10000000000ll,
    100000000000ll,
    1000000000000ll,
    10000000000000ll,
    100000000000000ll,
    1000000000000000ll,
    10000000000000000ll,
    100000000000000000ll,
    1000000000000000000ll
};

struct calcState
{
    double T, A, X;
    char Flag;
    int Shift;
    int Do;
    int Decimal;
    int CCount;
    struct uiLineEdit *Display;
};

void buttonMousePress(struct uiControl *sender, struct wmEvent *event)
{
    char *text = uiControlGetText(sender);
    struct calcState *state = (struct calcState *)uiControlGetContext(sender);

    if(text[0] >= '0' && text[0] <= '9')
    {
        state->CCount = 0;
        if(text[1] == '/')
        {
            state->T = 1.0 / state->T;
            state->Decimal = 0;
        }
        else
        {
            if(state->Shift)
            {
                state->A = state->T;
                state->T = 0;
                state->Decimal = 0;
                state->Shift = 0;
                state->Do = 1;
            }
            int digit = text[0] - '0';
            if(!state->Decimal)
                state->T = state->T * 10 + digit;
            else if(state->Decimal <= 10)
                state->T = state->T + digit / pow(10, state->Decimal++);
        }
    }
    else if(text[0] == 's')
    {
        state->CCount = 0;
        state->Decimal = 0;
        state->T = sqrt(state->T);
    }
    else if(text[0] == 'C')
    {
        if(!state->CCount)
        {
            state->T = 0;
            state->Shift = 0;
            state->Decimal = 0;
            ++state->CCount;
        }
        else
        {
            state->T = 0;
            state->A = 0;
            state->X = 0;
            state->Flag = 0;
            state->Shift = 0;
            state->CCount = 0;
            state->Decimal = 0;
        }
    }
    else if(text[0] == '.')
    {
        state->CCount = 0;
        state->Decimal = state->Decimal ? state->Decimal : 1;
    }
    else if(text[0] == '+' || text[0] == '-' || text[0] == '*' || text[0] == '/')
    {
        state->CCount = 0;
        state->Flag = text[0];
        state->Shift = 1;
    }
    else if(text[0] == '=')
    {
        state->CCount = 0;
        state->Decimal = 0;
        if(state->Do)
        {
            state->X = state->T;
            state->Do = 0;
        }
        switch(state->Flag)
        {
        case '+':
            state->A += state->X;
            state->T = state->A;
            break;
        case '-':
            state->A -= state->X;
            state->T = state->A;
            break;
        case '*':
            state->A *= state->X;
            state->T = state->A;
            break;
        case '/':
            state->A /= state->X;
            state->T = state->A;
            break;
        }
    }
    else if(text[0] == '%')
    {
        state->CCount = 0;
        state->Decimal = 0;
        if(state->Do)
        {
            state->X = state->T;
            state->Do = 0;
        }
        switch(state->Flag)
        {
        case '+':
            state->A += state->A * 0.01 * state->X;
            state->T = state->A;
            break;
        case '-':
            state->A -= state->A * 0.01 * state->X;
            state->T = state->A;
            break;
        case '*':
            state->A *= 0.01 * state->X;
            state->T = state->A;
            break;
        case '/':
            state->A /= 0.01 * state->X;
            state->T = state->A;
            break;
        }
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "%.6g", state->T);
    buf[sizeof(buf) - 1] = 0;
    uiControlSetText((struct uiControl *)state->Display, buf);
    uiControlRedraw((struct uiControl *)state->Display);
}

int main(int argc, char *argv[])
{
    wmInitialize();
    struct wmWindow *wnd = wmCreateWindow(400, 300, 200, 300, "Calculator", 1);
    struct pmPixMap *pm = wnd->ClientArea;
    wmShowWindow(wnd);

    struct calcState state;
    memset(&state, 0, sizeof(state));

    struct uiLineEdit *display = uiLineEditCreate(wnd->RootControl, 4, 4, wnd->ClientRectangle.Width - 8, 24, "0", NULL, NULL);
    state.Display = display;
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
            uiControlSetContext((struct uiControl *)buttons[btn], &state);
            uiControlSetOnMousePress((struct uiControl *)buttons[btn], buttonMousePress);
        }
    }

    uiControlRedraw(wnd->RootControl);
    wmUpdateWindow(wnd);

    for(;;)
    {
        struct wmEvent event;
        wmGetEvent(wnd, &event);
        wmProcessEvent(wnd, &event);
        if(event.Type == ET_KEYBOARD && event.Keyboard.Flags == 0)
        {
            if(event.Keyboard.Key == VK_ESCAPE)
                break;
            else if(event.Keyboard.Key == VK_KEY0 || event.Keyboard.Key == VK_NUMPAD0)
                buttonMousePress((struct uiControl *)buttons[16], &event);
            else if(event.Keyboard.Key == VK_KEY1 || event.Keyboard.Key == VK_NUMPAD1)
                buttonMousePress((struct uiControl *)buttons[12], &event);
            else if(event.Keyboard.Key == VK_KEY2 || event.Keyboard.Key == VK_NUMPAD2)
                buttonMousePress((struct uiControl *)buttons[13], &event);
            else if(event.Keyboard.Key == VK_KEY3 || event.Keyboard.Key == VK_NUMPAD3)
                buttonMousePress((struct uiControl *)buttons[14], &event);
            else if(event.Keyboard.Key == VK_KEY4 || event.Keyboard.Key == VK_NUMPAD4)
                buttonMousePress((struct uiControl *)buttons[8], &event);
            else if(event.Keyboard.Key == VK_KEY5 || event.Keyboard.Key == VK_NUMPAD5)
                buttonMousePress((struct uiControl *)buttons[9], &event);
            else if(event.Keyboard.Key == VK_KEY6 || event.Keyboard.Key == VK_NUMPAD6)
                buttonMousePress((struct uiControl *)buttons[10], &event);
            else if(event.Keyboard.Key == VK_KEY7 || event.Keyboard.Key == VK_NUMPAD7)
                buttonMousePress((struct uiControl *)buttons[4], &event);
            else if(event.Keyboard.Key == VK_KEY8 || event.Keyboard.Key == VK_NUMPAD8)
                buttonMousePress((struct uiControl *)buttons[5], &event);
            else if(event.Keyboard.Key == VK_KEY9 || event.Keyboard.Key == VK_NUMPAD9)
                buttonMousePress((struct uiControl *)buttons[6], &event);
            else if(event.Keyboard.Key == VK_OEMPERIOD || event.Keyboard.Key == VK_DECIMAL)
                buttonMousePress((struct uiControl *)buttons[17], &event);
            else if(event.Keyboard.Key == VK_RETURN)
                buttonMousePress((struct uiControl *)buttons[18], &event);
            else if(event.Keyboard.Key == VK_BACK || event.Keyboard.Key == VK_DELETE)
                buttonMousePress((struct uiControl *)buttons[0], &event);
            else if(event.Keyboard.Key == VK_OEMPLUS || event.Keyboard.Key == VK_ADD)
                buttonMousePress((struct uiControl *)buttons[19], &event);
            else if(event.Keyboard.Key == VK_OEMMINUS || event.Keyboard.Key == VK_SUBTRACT)
                buttonMousePress((struct uiControl *)buttons[15], &event);
            else if(event.Keyboard.Key == VK_MULTIPLY)
                buttonMousePress((struct uiControl *)buttons[11], &event);
            else if(event.Keyboard.Key == VK_DIVIDE)
                buttonMousePress((struct uiControl *)buttons[7], &event);
            else if(event.Keyboard.Key == VK_F12)
                wmRedrawWindow(wnd);
        }
    }

    wmDestroyWindow(wnd->ID);
    wmCleanup();
    return 0;
}
