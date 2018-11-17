#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <woot/wm.h>

int main(int argc, char *argv[])
{
    int wnd = wmCreateWindow(50, 150, 400, 300);
    for(;;);
    struct wmRectangle rect = {0, 0, 400, 24};
    wmDrawFilledRectangle(wnd, &rect , 0x40608000);
    rect.Height = 300;
    wmDrawRectangle(wnd, &rect, 0xFFFFFF00);
    wmShowWindow(wnd);

    printf("WOOT test user mode console\n");

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

        malloc(100);

        int _argc = 0;
        for(char *it = buf, *token; (token = strtok_r(it, " \t", &it));)
            _argv[_argc++] = token;
        if(!_argc) continue;

        if(!strcmp(_argv[0], "quit") || !strcmp(_argv[0], "exit"))
            break;
        else if(!strcmp(_argv[0], "mstat"))
            malloc_stats();
        else if(!strcmp(_argv[0], "time"))
        {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            printf("%.2d:%.2d:%.2d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
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
