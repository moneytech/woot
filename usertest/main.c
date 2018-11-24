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

    int wnd = wmCreateWindow(50, 450, w, h);
    struct wmRectangle rect = {0, 0, w, 24};
    wmDrawFilledRectangle(wnd, &rect , 0x40608000);
    rect.Height = h;
    wmDrawRectangle(wnd, &rect, 0xFFFFFF00);
    wmShowWindow(wnd);

    printf("WOOT test user mode console\n");
    printf("main at: %p\n", main);
    printf("exit at: %p\n", exit);
    printf("wmCreateWindow at: %p\n", wmCreateWindow);
    printf("zlib version: %s\n", zlibVersion());
    printf("libpng version: %s\n", png_get_libpng_ver(NULL));
    FT_Library freetype;
    FT_Init_FreeType(&freetype);
    FT_Int ftmajor, ftminor, ftpatch;
    FT_Library_Version(freetype, &ftmajor, &ftminor, &ftpatch);
    printf("freetype version: %d.%d.%d\n", ftmajor, ftminor, ftpatch);

    FT_Face face;
    FT_Error err;
    err = FT_New_Face(freetype, "WOOT_OS:/test.ttf", 0, &face);
    if(err) printf("FT_New_Face failed: %d\n", err);
    else
    {
        printf("font: %s\n", face->family_name);
        err = FT_Set_Char_Size(
                    face,      /* handle to face object           */
                    0,         /* char_width in 1/64th of points  */
                    64 * 64,   /* char_height in 1/64th of points */
                    0,         /* horizontal device resolution    */
                    96);       /* vertical device resolution      */
    }

    struct pmPixMap *dirIcon = pmLoadPNG("WOOT_OS:/directory.png");
    struct pmPixMap *fileIcon = pmLoadPNG("WOOT_OS:/file.png");

    struct pmPixMap *pm = wmWindowToPixMap(wnd);
    struct pmPixMap *spm = pmSubPixMap(pm, 1, 24, pm->Width - 2, pm->Height - 25);

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
                wmUpdateWindow(0);
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

                pmClear(spm, pmColorFromRGB(0, 0, 255));
                wmInvalidateRectangle(wnd, NULL);

                int cx = spm->Width / 2;
                int cy = spm->Height / 2;
                int sz = min(cx, cy);
                for(int i = 0; i < 12; ++i)
                {
                    double angle = i * (2 * M_PI / 12);
                    double s = sin(angle);
                    double c = -cos(angle);
                    pmLine(spm, cx + s * sz * 0.8, cy + c * sz * 0.8, cx + s * sz * 0.9, cy + c * sz * 0.9, pmColorWhite);
                }

                // second hand
                double angle = tm->tm_sec * (2 * M_PI / 60);
                double s = sin(angle);
                double c = -cos(angle);
                pmLine(spm, cx + s * sz * -0.2, cy + c * sz * -0.2, cx + s * sz * 0.75, cy + c * sz * 0.75, pmColorWhite);

                // minute hand
                angle = tm->tm_min * (2 * M_PI / 60);
                s = sin(angle);
                c = -cos(angle);
                pmLine(spm, cx + s * sz * -0.15, cy + c * sz * -0.15, cx + -c * sz * 0.05, cy + s * sz * 0.05, pmColorWhite);
                pmLine(spm, cx + -c * sz * 0.05, cy + s * sz * 0.05, cx + s * sz * 0.6, cy + c * sz * 0.6, pmColorWhite);
                pmLine(spm, cx + s * sz * -0.15, cy + c * sz * -0.15, cx + c * sz * 0.05, cy + -s * sz * 0.05, pmColorWhite);
                pmLine(spm, cx + c * sz * 0.05, cy + -s * sz * 0.05, cx + s * sz * 0.6, cy + c * sz * 0.6, pmColorWhite);

                // hour hand
                angle = tm->tm_hour * (2 * M_PI / 12);
                s = sin(angle);
                c = -cos(angle);
                pmLine(spm, cx + s * sz * -0.1, cy + c * sz * -0.1, cx + -c * sz * 0.05, cy + s * sz * 0.05, pmColorWhite);
                pmLine(spm, cx + -c * sz * 0.05, cy + s * sz * 0.05, cx + s * sz * 0.5, cy + c * sz * 0.5, pmColorWhite);
                pmLine(spm, cx + s * sz * -0.1, cy + c * sz * -0.1, cx + c * sz * 0.05, cy + -s * sz * 0.05, pmColorWhite);
                pmLine(spm, cx + c * sz * 0.05, cy + -s * sz * 0.05, cx + s * sz * 0.5, cy + c * sz * 0.5, pmColorWhite);

                wmUpdateWindow(wnd);
                struct timespec ts = { 0, 100 * 1000000 };
                nanosleep(&ts, NULL);
            }
        }
        else if(!strcmp(_argv[0], "dir"))
        {
            pmAlphaBlit(spm, dirIcon, 0, 0, 0, 0, dirIcon->Width, dirIcon->Height);
            pmAlphaBlit(spm, fileIcon, 0, 0, 48, 0, fileIcon->Width, fileIcon->Height);
            wmRedrawWindow(wnd);
        }
        else if(!strcmp(_argv[0], "date"))
        {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            printf("%.4d-%.2d-%.2d\n", tm->tm_year, tm->tm_mon, tm->tm_mday);
        }
        else printf("unknown command '%s'\n", _argv[0]);
    }

    wmDestroyWindow(wnd);

    return 42;
}

