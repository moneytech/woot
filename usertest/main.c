#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H

#include <woot/pixmap.h>
#include <woot/wm.h>
#include <zlib.h>
#include <png.h>

// For some weird reason libpng need these symbols
// like it has something to do with profiling

void __cxa_finalize()
{
}

void _ITM_registerTMCloneTable()
{
}

void _ITM_deregisterTMCloneTable()
{
}

void __gmon_start__()
{
}

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

int main(int argc, char *argv[])
{
    int w = 400, h = 300;

    wmInitialize();
    struct wmWindow *wnd = wmCreateWindow(50, 450, w, h, "Test usermode window", 1);
    wmShowWindow(wnd);

    printf("WOOT test user mode console\n");

    struct pmPixMap *dirIcon = pmLoadPNG("WOOT_OS:/directory.png");
    struct pmPixMap *fileIcon = pmLoadPNG("WOOT_OS:/file.png");

    struct pmPixMap *spm = wnd->ClientArea;

    char buf[128];
    char *_argv[64];
    for(;;)
    {
        getcwd(buf, sizeof(buf));
        printf("%s# ", buf);
        int br = fread(buf, 1, sizeof(buf) - 1, stdin);
        char *nl = strrchr(buf, '\n');
        if(nl) *nl = 0;
        buf[br] = 0;

        int _argc = 0;
        for(char *it = buf, *token; (token = strtok_r(it, " \t", &it));)
            _argv[_argc++] = token;
        if(!_argc) continue;

        if(!strcmp(_argv[0], "quit") || !strcmp(_argv[0], "exit"))
            break;
        else if(!strcmp(_argv[0], "args"))
        {
            for(int i = 0; i < argc; ++i)
                printf("%d: %s\n", i, argv[i]);
        }
        else if(!strcmp(_argv[0], "blit"))
        {
            struct pmPixMap *pm = pmLoadPNG("WOOT_OS:/wallpaper.png");
            if(pm)
            {
                wmBlit(0, pm, 0, 0, 0, 0, pm->Width, pm->Height);
                pmDelete(pm);
                wmUpdateWindowByID(0);
            }
        }
        else if(!strcmp(_argv[0], "mstat"))
            malloc_stats();
        else if(!strcmp(_argv[0], "time"))
        {
            for(int i = 0; i < 100; ++i)
            {
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                if(!i) printf("%.2d:%.2d:%.2d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);

                pmClear(spm, pmColorGray);
                wmInvalidateRectangle(wnd, &wnd->ClientRectangle);

                int cx = spm->Width / 2;
                int cy = spm->Height / 2;
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
        else if(!strcmp(_argv[0], "dir"))
        {
            pmAlphaBlit(spm, dirIcon, 0, 0, 0, 0, dirIcon->Width, dirIcon->Height);
            pmAlphaBlit(spm, fileIcon, 0, 0, 48, 0, fileIcon->Width, fileIcon->Height);
            wmInvalidateRectangle(wnd, &wnd->ClientRectangle);
            wmUpdateWindow(wnd);
        }
        else if(!strcmp(_argv[0], "date"))
        {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            printf("%.4d-%.2d-%.2d\n", tm->tm_year, tm->tm_mon, tm->tm_mday);
        }
        else printf("unknown command '%s'\n", _argv[0]);
    }

    wmDeleteWindow(wnd);
    wmCleanup();

    return 42;
}

