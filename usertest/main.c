#include <dirent.h>
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
#include <woot/thread.h>
#include <woot/time.h>
#include <woot/wm.h>
#include <zlib.h>
#include <png.h>

#include <SDL/SDL.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

void testThread()
{
    int tid = thrGetCurrentID();
    for(int i = 0;; ++i)
    {
        printf("t%d: %d\n", tid, i);
        tmSleep(1000);
    }
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
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
    for(; 1;)
    {
        getcwd(buf, sizeof(buf));
        printf("%s# ", buf);
        int br = read(0, buf, sizeof(buf) - 1);
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
        else if(!strcmp(_argv[0], "gettid"))
            printf("current thread id: %d\n", thrGetCurrentID());
        else if(!strcmp(_argv[0], "cthread"))
        {
            int id = thrCreate(testThread);
            printf("%d\n", id);
            if(id >= 0)
                thrResume(id);
        }
        else if(!strcmp(_argv[0], "tsusp"))
        {
            if(!_argv[1]) printf("missing thread id\n");
            else
            {
                int id = atoi(_argv[1]);
                thrSuspend(id);
            }
        }
        else if(!strcmp(_argv[0], "tresm"))
        {
            if(!_argv[1]) printf("missing thread id\n");
            else
            {
                int id = atoi(_argv[1]);
                thrResume(id);
            }
        }
        else if(!strcmp(_argv[0], "tsleep"))
        {
            if(!_argv[1]) printf("missing thread id\n");
            else if(!_argv[2]) printf("missing milliseconds\n");
            else
            {
                int id = atoi(_argv[1]);
                int ms = atoi(_argv[2]);
                thrSleep(id, ms);
            }
        }
        else if(!strcmp(_argv[0], "sdl"))
        {
            SDL_Init(0);
            SDL_Surface *surf = SDL_SetVideoMode(640, 480, 8, 0);
            uint8_t *pixels = (uint8_t *)surf->pixels;
            uint32_t color = 0;
            if(surf)
            {
                SDL_Event event;
                while(SDL_WaitEvent(&event) >= 0)
                {
                    int quit = 0;
                    switch(event.type)
                    {
                    case SDL_QUIT:
                        quit = 1;
                        break;
                    case SDL_KEYDOWN:
                        switch(event.key.keysym.sym)
                        {
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                        case SDLK_SPACE:
                            color = rand() ^ (rand() << 10) ^ (rand() << 20);
                            for(int y = 0; y < surf->h; ++y)
                            {
                                uint8_t *line = (uint8_t *)(pixels + y * surf->pitch);
                                for(int x = 0; x < surf->w; ++x)
                                    line[x] = x ^ y + color;
                            }
                            break;
                        }
                        break;
                    }
                    if(quit) break;
                    printf(".");
                    SDL_Flip(surf);
                }
            } else printf("error: %s\n", SDL_GetError());
            SDL_Quit();
        }
        else if(!strcmp(_argv[0], "dir"))
        {
            char *name = _argc >= 2 ? _argv[1] : ".";
            DIR *dir = opendir(name);
            if(!dir) printf("couldn't open directory %s\n", name);
            else
            {
                struct fntFont *font = wmGetDefaultFont();
                float fh = fntGetPixelHeight(font);
                struct dirent *de;
                int i = 0;
                while((de = readdir(dir)))
                {
                    int isDir = de->d_type == DT_DIR;
                    struct pmPixMap *icon = isDir ? dirIcon : fileIcon;
                    struct wmRectangle rect = pmGetRectangle(icon);
                    pmAlphaBlit(spm, icon, 0, 0, 12, i * 48, rect.Width, rect.Height);                    
                    fntDrawString(font, spm, 72, i * 48 + (48 - fh) / 2, de->d_name, pmColorBlack);
                    rect = pmGetRectangle(spm);
                    pmDrawFrame(spm, 0, i * 48, rect.Width, 48, 0);
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

