#include <dirent.h>
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

#include <woot/font.h>
#include <woot/pixmap.h>
#include <woot/wm.h>
#include <zlib.h>
#include <png.h>

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
        else if(!strcmp(_argv[0], "mstat"))
            malloc_stats();
        else if(!strcmp(_argv[0], "dir"))
        {
            char *name = _argc >= 2 ? _argv[1] : ".";
            DIR *dir = opendir(name);
            if(!dir) printf("couldn't open directory %s\n", name);
            else
            {
                struct dirent *de;
                int i = 0;
                while((de = readdir(dir)))
                {
                    int isDir = de->d_type == DT_DIR;
                    struct pmPixMap *icon = isDir ? dirIcon : fileIcon;
                    pmAlphaBlit(spm, icon, 0, 0, 12, i * 48, icon->Width, icon->Height);
                    fntDrawString(wmGetDefaultFont(), spm, 72, i * 48 + 32, de->d_name, pmColorBlack);
                    pmDrawFrame(spm, 0, i * 48, spm->Width, 48, 0);
                    //printf("%-16s %s\n", de->d_name, isDir ? "<DIR>" : "     ");
                    ++i;
                }
                wmInvalidateRectangle(wnd, &wnd->ClientRectangle);
                wmUpdateWindow(wnd);
            }
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

