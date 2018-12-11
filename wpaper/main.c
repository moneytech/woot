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
    rect = pmGetRectangle(pm);
    wmBlit(0, pm, 0, 0, (w - rect.Width) / 2, (h - rect.Height) / 2, rect.Width, rect.Height);
    pmDelete(pm);
    wmUpdateWindowByID(0);
    return 0;
}
