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

const int maxDecPlaces = 6;

struct calcState
{
    // gmp will come in handy here
    long long T, A, X;
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
            state->T = pow10Table[maxDecPlaces + 1] / state->T;
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
                state->T = state->T * 10 + digit * pow10Table[maxDecPlaces];
            else if(state->Decimal <= maxDecPlaces)
                state->T = state->T + digit * pow10Table[maxDecPlaces - state->Decimal++];
        }
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
            state->A /= pow10Table[maxDecPlaces];
            state->T = state->A;
            break;
        case '/':
            state->A = (state->A * pow10Table[maxDecPlaces]) / state->X;
            state->T = state->A;
            break;
        default:
            state->A = 0;
            state->T = state->A;
            break;
        }
    }

    char buf[64];
    int decPlaces = 0;
    long long integ = state->T / pow10Table[maxDecPlaces];
    long long frac = state->T % pow10Table[maxDecPlaces];
    for(; decPlaces < maxDecPlaces && !(frac % pow10Table[decPlaces]); ++decPlaces);
    decPlaces = 1 + maxDecPlaces - decPlaces;
    snprintf(buf, sizeof(buf), "%lld.%lld", integ, frac / pow10Table[maxDecPlaces - decPlaces]);
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
