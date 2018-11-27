#include <math.h>
#include <stdio.h>
#include <time.h>
#include <woot/font.h>
#include <woot/pixmap.h>
#include <woot/wm.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

int main(int argc, char *argv[])
{
    wmInitialize();
    struct wmWindow *wnd = wmCreateWindow(100, 100, 200, 200, "Clock", 1);
    if(!wnd) return -1;
    wmShowWindow(wnd);

    char buf[64];
    struct pmPixMap *spm = wnd->ClientArea;
    struct fntFont *font = wmGetDefaultFont();
    if(spm && font)
    {
        for(int i = 0;; ++i)
        {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            sprintf(buf, "%.2d:%.2d:%.2d", tm->tm_hour, tm->tm_min, tm->tm_sec);
            int timeWidth = fntMeasureString(font, buf);

            pmClear(spm, pmColorGray);
            wmInvalidateRectangle(wnd, &wnd->ClientRectangle);

            int cx = spm->Width / 2;
            int cy = spm->Height / 2;

            fntDrawString(font, spm, 1 + cx - timeWidth / 2, 1 + cy + 32, buf, pmColorBlack);
            fntDrawString(font, spm, cx - timeWidth / 2, cy + 32, buf, pmColorWhite);

            int sz = min(cx, cy);
            for(int i = 0; i < 12; ++i)
            {
                double angle = i * (2 * M_PI / 12);
                double s = sin(angle);
                double c = -cos(angle);
                pmLine(spm, cx + s * sz * 0.8, cy + c * sz * 0.8, cx + s * sz * 0.9, cy + c * sz * 0.9, pmColorBlack);
            }

            // second hand
            double angle = tm->tm_sec * (2 * M_PI / 60);
            double s = sin(angle);
            double c = -cos(angle);
            pmLine(spm, cx + s * sz * -0.2, cy + c * sz * -0.2, cx + s * sz * 0.75, cy + c * sz * 0.75, pmColorBlack);

            // minute hand
            angle = tm->tm_min * (2 * M_PI / 60);
            s = sin(angle);
            c = -cos(angle);
            pmLine(spm, cx + s * sz * -0.15, cy + c * sz * -0.15, cx + -c * sz * 0.05, cy + s * sz * 0.05, pmColorBlack);
            pmLine(spm, cx + -c * sz * 0.05, cy + s * sz * 0.05, cx + s * sz * 0.6, cy + c * sz * 0.6, pmColorBlack);
            pmLine(spm, cx + s * sz * -0.15, cy + c * sz * -0.15, cx + c * sz * 0.05, cy + -s * sz * 0.05, pmColorBlack);
            pmLine(spm, cx + c * sz * 0.05, cy + -s * sz * 0.05, cx + s * sz * 0.6, cy + c * sz * 0.6, pmColorBlack);

            // hour hand
            angle = tm->tm_hour * (2 * M_PI / 12);
            s = sin(angle);
            c = -cos(angle);
            pmLine(spm, cx + s * sz * -0.1, cy + c * sz * -0.1, cx + -c * sz * 0.05, cy + s * sz * 0.05, pmColorBlack);
            pmLine(spm, cx + -c * sz * 0.05, cy + s * sz * 0.05, cx + s * sz * 0.5, cy + c * sz * 0.5, pmColorBlack);
            pmLine(spm, cx + s * sz * -0.1, cy + c * sz * -0.1, cx + c * sz * 0.05, cy + -s * sz * 0.05, pmColorBlack);
            pmLine(spm, cx + c * sz * 0.05, cy + -s * sz * 0.05, cx + s * sz * 0.5, cy + c * sz * 0.5, pmColorBlack);

            wmUpdateWindow(wnd);
            struct timespec ts = { 0, 100 * 1000000 };
            nanosleep(&ts, NULL);
        }
    }

    wmDeleteWindow(wnd);
    wmCleanup();
}
