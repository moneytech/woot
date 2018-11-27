#include <errno.h>
#include <stdio.h>
#include <woot/pixmap.h>
#include <woot/wm.h>

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("sets wallpaper\n");
        printf("usage: %s <filename>\n", argv[0]);
        return -EINVAL;
    }
    struct pmPixMap *pm = pmLoadPNG(argv[1]);
    if(!pm)
    {
        printf("couldn't open %s\n", argv[2]);
        return -ENOENT;
    }
    int w, h;
    wmGetWindowSize(0, &w, &h);
    struct wmRectangle rect = { 0, 0, w, h };
    wmDrawFilledRectangle(0, &rect, pmColorDarkGray.Value);
    wmBlit(0, pm, 0, 0, (w - pm->Width) / 2, (h - pm->Height) / 2, pm->Width, pm->Height);
    pmDelete(pm);
    wmUpdateWindowByID(0);
    return 0;
}
