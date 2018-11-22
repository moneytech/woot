#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

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
            png_bytepp image;
            int width = 640, height = 480;
            int pngok = 0;
            int pitch = 0;
            png_byte color_type;
            png_byte bpp;
            FILE *f = fopen("WOOT_OS:/wallpaper.png", "rb");
            if(f)
            {
                png_byte header[8];
                fread(header, sizeof(header), 1, f);
                if(!png_sig_cmp(header, 0, sizeof(header)))
                {
                    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                    png_infop info = png_create_info_struct(png);
                    png_init_io(png, f);
                    png_set_sig_bytes(png, sizeof(header));
                    png_read_info(png, info);
                    width = png_get_image_width(png, info);
                    height = png_get_image_height(png, info);
                    color_type = png_get_color_type(png, info);
                    bpp = png_get_bit_depth(png, info);
                    printf("w: %d h: %d bpp: %d\n", width, height, bpp);
                    png_set_interlace_handling(png);
                    png_read_update_info(png, info);
                    image = (png_bytepp)calloc(height, sizeof(png_bytep));
                    pitch = png_get_rowbytes(png, info);
                    for(int y = 0; y < height; y++)
                        image[y] = (png_bytep)calloc(1, pitch);
                    png_read_image(png, image);
                    pngok = 1;
                }
                fclose(f);
            } else printf("couldn't open wallpaper\n");

            if(pngok && bpp == 8 && (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA))
            {
                struct pmPixelFormat pf = { 32, 0, 0, 8, 16, 0, 8, 8, 8 };
                struct pmPixMap *pm = pmCreate(width, height, pf);
                for(int i = 0; i < height; ++i)
                    memcpy(pm->PixelBytes + i * pm->Pitch, image[i], min(pm->Pitch, pitch));
                wmBlit(0, pm, 0, 0, 0, 0, width, height);
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

                struct wmRectangle rect = { 1, 25, w - 2, h - 26 };
                wmDrawFilledRectangle(wnd, &rect, 0x80000000);

                int cx = w / 2;
                int cy = ((h - 24) / 2) + 24;
                int sz = min(cx, cy - 24);
                for(int i = 0; i < 12; ++i)
                {
                    double angle = i * (2 * M_PI / 12);
                    double s = sin(angle);
                    double c = -cos(angle);
                    wmDrawLine(wnd,
                               cx + s * sz * 0.8,
                               cy + c * sz * 0.8,
                               cx + s * sz * 0.9,
                               cy + c * sz * 0.9,
                               0xFFFFFF00);
                }

                // second hand
                double angle = tm->tm_sec * (2 * M_PI / 60);
                double s = sin(angle);
                double c = -cos(angle);
                wmDrawLine(wnd,
                           cx + s * sz * -0.2,
                           cy + c * sz * -0.2,
                           cx + s * sz * 0.75,
                           cy + c * sz * 0.75,
                           0xFFFFFF00);

                // minute hand
                angle = tm->tm_min * (2 * M_PI / 60);
                s = sin(angle);
                c = -cos(angle);
                wmDrawLine(wnd,
                           cx + s * sz * -0.15,
                           cy + c * sz * -0.15,
                           cx + -c * sz * 0.05,
                           cy + s * sz * 0.05,
                           0xFFFFFF00);
                wmDrawLine(wnd,
                           cx + -c * sz * 0.05,
                           cy + s * sz * 0.05,
                           cx + s * sz * 0.6,
                           cy + c * sz * 0.6,
                           0xFFFFFF00);
                wmDrawLine(wnd,
                           cx + s * sz * -0.15,
                           cy + c * sz * -0.15,
                           cx + c * sz * 0.05,
                           cy + -s * sz * 0.05,
                           0xFFFFFF00);
                wmDrawLine(wnd,
                           cx + c * sz * 0.05,
                           cy + -s * sz * 0.05,
                           cx + s * sz * 0.6,
                           cy + c * sz * 0.6,
                           0xFFFFFF00);

                // hour hand
                angle = tm->tm_hour * (2 * M_PI / 12);
                s = sin(angle);
                c = -cos(angle);
                wmDrawLine(wnd,
                           cx + s * sz * -0.1,
                           cy + c * sz * -0.1,
                           cx + -c * sz * 0.05,
                           cy + s * sz * 0.05,
                           0xFFFFFF00);
                wmDrawLine(wnd,
                           cx + -c * sz * 0.05,
                           cy + s * sz * 0.05,
                           cx + s * sz * 0.5,
                           cy + c * sz * 0.5,
                           0xFFFFFF00);
                wmDrawLine(wnd,
                           cx + s * sz * -0.1,
                           cy + c * sz * -0.1,
                           cx + c * sz * 0.05,
                           cy + -s * sz * 0.05,
                           0xFFFFFF00);
                wmDrawLine(wnd,
                           cx + c * sz * 0.05,
                           cy + -s * sz * 0.05,
                           cx + s * sz * 0.5,
                           cy + c * sz * 0.5,
                           0xFFFFFF00);

                wmUpdateWindow(wnd);
                struct timespec ts = { 0, 100 * 1000000 };
                nanosleep(&ts, NULL);
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

    wmDestroyWindow(wnd);

    return 42;
}
