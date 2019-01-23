#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <woot/audio.h>
#include <woot/pixmap.h>
#include <woot/ui.h>
#include <woot/vkeys.h>
#include <woot/wm.h>

static int mapValue(int imin, int imax, int omin, int omax, int val)
{
    return (float)(val - imin) / (imax - imin) * (omax - omin) + omin;
}

int main(int argc, char *argv[])
{

    if(argc < 2)
    {
        printf("WOOT sound file player\n");
        printf("usage: %s <filename> [device_id]\n", argv[0]);
        return 0;
    }

    FILE *f = fopen(argv[1], "rb");
    if(!f)
    {
        printf("Couldn't open '%s'\n", argv[1]);
        return -ENOENT;
    }

    // very simple and dodgy wave header parsing
    uint32_t riff = 0;
    int res = fread(&riff, 4, 1, f);
    if(res != 1)
    {
        printf("Couldn't read RIFF header\n");
        fclose(f);
        return -EIO;
    }
    if(riff != 0x46464952)
    {
        printf("Invalid RIFF header\n");
        fclose(f);
        return -EINVAL;
    }

    fseek(f, 4, SEEK_CUR);
    uint32_t wave = 0;
    res = fread(&wave, 4, 1, f);
    if(res != 1)
    {
        printf("Couldn't read WAVE format id\n");
        fclose(f);
        return -EIO;
    }
    if(wave != 0x45564157)
    {
        printf("Invalid WAVE format id\n");
        fclose(f);
        return -EINVAL;
    }

    uint32_t fmt_ = 0;
    res = fread(&fmt_, 4, 1, f);
    if(res != 1)
    {
        printf("Couldn't read subchunk 1 id id\n");
        fclose(f);
        return -EIO;
    }
    if(fmt_ != 0x20746d66)
    {
        printf("Invalid subchunk 1 id\n");
        fclose(f);
        return -EINVAL;
    }

    int rate = 22050;
    int channels = 1;
    int bits = 8;
    int samples = 2048;

    fseek(f, 6, SEEK_CUR);
    fread(&channels, 2, 1, f);
    fread(&rate, 4, 1, f);
    fseek(f, 6, SEEK_CUR);
    fread(&bits, 2, 1, f);
    fseek(f, 6, SEEK_CUR);

    int devId = 0;
    if(argc >= 3) devId = strtoul(argv[2], NULL, 0);

    printf("Opening device %d: %d Hz %d channel(s) %d bits per sample\n", devId, rate, channels, bits);

    res = auOpenDevice(devId, rate, channels, bits, samples);
    if(res)
    {
        printf("Couldn't open device %d\n", devId);
        fclose(f);
        return res;
    }

    int frameSize = auGetFrameSize(devId);
    if(frameSize < 0)
    {
        printf("Couldn't get frame size\n");
        auCloseDevice(devId);
        fclose(f);
        return frameSize;
    }

    char title[128];
    char vendor[32];
    char model[32];
    auGetDeviceVendor(devId, vendor, sizeof(vendor));
    auGetDeviceModel(devId, model, sizeof(model));
    sprintf(title, "%s - %s (on %s %s)", "SndPlay", argv[1], vendor, model);

    union
    {
        void *ptr;
        uint8_t *byte;
        int16_t *sshort;
    } buffer;

    buffer.ptr = malloc(frameSize);
    int bufferCount = auGetBufferCount(devId);

    wmInitialize();
    struct wmWindow *wnd = wmCreateWindow(200, 200, 400, 150, title, 1);
    wmShowWindow(wnd);

    struct uiLabel *lblPosition = uiLabelCreate(wnd->RootControl, 0, wnd->ClientRectangle.Height - 24, 48, 24, "00:00", NULL, NULL);
    struct uiButton *btnPlay = uiButtonCreate(wnd->RootControl, 48, wnd->ClientRectangle.Height - 24, 24, 24, ">", NULL, NULL);
    struct uiButton *btnPause = uiButtonCreate(wnd->RootControl, 72, wnd->ClientRectangle.Height - 24, 24, 24, "||", NULL, NULL);
    struct uiButton *btnStop = uiButtonCreate(wnd->RootControl, 96, wnd->ClientRectangle.Height - 24, 24, 24, "\xFF", NULL, NULL);

    struct pmPixMap *pm = wnd->ClientArea;
    struct wmRectangle pmRect = pmGetRectangle(pm);
    struct wmRectangle dispRect = { pmRect.X, pmRect.Y, pmRect.Width, pmRect.Height - 24 };

    uiControlRedraw(wnd->RootControl);
    wmUpdateWindow(wnd);

    struct wmEvent event;
    for(int frame = 0, res = 0;; ++frame)
    {
        int quit = 0;
        while(wmPeekEvent(wnd, &event, 1) > 0)
        {
            res = wmProcessEvent(wnd, &event);
            if(res)
            {
                quit = 1;
                break;
            }
            if(event.Type == ET_KEYBOARD && event.Keyboard.Flags == 0)
            {
                if(event.Keyboard.Key == VK_ESCAPE)
                {
                    quit = 1;
                    break;
                }
            }
        }
        if(quit) break;

        pmFillRectangle(pm, 0, 0, dispRect.Width, dispRect.Height, pmColorBlack);
        pmInvalidateRect(pm, dispRect);
        int cy = dispRect.Height / 2;
        for(int x = 0; x < wnd->ClientRectangle.Width; ++x)
        {
            int val = 0;
            if(bits == 8 && channels == 1)
                val = buffer.byte[mapValue(0, wnd->ClientRectangle.Width, 0, samples, x)] - 128;
            else if(bits == 8 && channels == 2)
                val = buffer.byte[2 * mapValue(0, wnd->ClientRectangle.Width, 0, samples, x)] - 128;
            else if(bits == 16 && channels == 1)
                val = buffer.sshort[mapValue(0, wnd->ClientRectangle.Width, 0, samples, x)] >> 8;
            else if(bits == 16 && channels == 2)
                val = buffer.sshort[2 * mapValue(0, wnd->ClientRectangle.Width, 0, samples, x)] >> 8;

            val = mapValue(-128, 127, -cy, cy, val);
            pmVLine(pm, x, cy, cy + val, pmColorBrightGreen);
        }
        wmUpdateWindow(wnd);

        res = fread(buffer.ptr, 1, frameSize, f);
        if(res < 0)
        {
            printf("Read error\n");
            break;
        }
        else if(res != frameSize)
            memset(buffer.byte + res, 0, frameSize - res);
        if(auWriteDevice(devId, buffer.byte))
        {
            printf("Playback error\n");
            break;
        }

        if(frame == (bufferCount - 1))
            auStartPlayback(devId);
        if(res != frameSize)
        {
            res = 0;
            break;
        }
    }
    auStopPlayback(devId);
    wmDeleteWindow(wnd);
    wmCleanup();
    free(buffer.ptr);
    auCloseDevice(devId);
    fclose(f);
    return res;
}

