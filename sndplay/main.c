#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <woot/audio.h>
#include <woot/pixmap.h>
#include <woot/ui.h>
#include <woot/vkeys.h>
#include <woot/wm.h>

struct PlayerContext
{
    int Rate;
    int Channels;
    int Bits;
    int BufferSamples;
    int TotalSamples;
    int Position;
    int BytesPerSample;

    union
    {
        void *Pointer;
        uint8_t *Bytes;
        int16_t *SWords;
    } Buffer;
    int Frame;
};

static int mapValue(int imin, int imax, int omin, int omax, int val)
{
    return (float)(val - imin) / (imax - imin) * (omax - omin) + omin;
}

void ctrlDisplayOnPaint(struct uiControl *sender)
{
    struct PlayerContext *context = (struct PlayerContext *)uiControlGetContext(sender);
    struct pmPixMap *pm = uiControlGetPixMap(sender);
    struct wmRectangle rect = pmGetRectangle(pm);

    pmClear(pm, pmColorBlack);
    int cy = rect.Height / 2;
    for(int x = 0; x < rect.Width; ++x)
    {
        int val = 0;
        if(context->Bits == 8) val = context->Buffer.Bytes[context->Channels * mapValue(0, rect.Width, 0, context->BufferSamples, x)] - 128;
        else if(context->Bits == 16) val = context->Buffer.SWords[context->Channels * mapValue(0, rect.Width, 0, context->BufferSamples, x)] >> 8;
        val = mapValue(-128, 127, -cy, cy, val);
        pmVLine(pm, x, cy, cy + val, pmColorBrightGreen);
    }
    pmInvalidateRect(pm, rect);
}

int main(int argc, char *argv[])
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

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

    struct PlayerContext context;
    memset(&context, 0, sizeof(context));
    context.BufferSamples = 1024;

    fseek(f, 6, SEEK_CUR);
    fread(&context.Channels, 2, 1, f);
    fread(&context.Rate, 4, 1, f);
    fseek(f, 6, SEEK_CUR);
    fread(&context.Bits, 2, 1, f);
    fseek(f, 4, SEEK_CUR);
    fread(&context.TotalSamples, 4, 1, f);
    context.BytesPerSample = context.Channels * (context.Bits / 8);
    context.TotalSamples /= context.BytesPerSample;

    float secondsPerSample = 1.0f / context.Rate;

    int devId = 0;
    if(argc >= 3) devId = strtoul(argv[2], NULL, 0);

    printf("Opening device %d: %d Hz %d channel(s) %d bits per sample\n", devId, context.Rate, context.Channels, context.Bits);

    res = auOpenDevice(devId, context.Rate, context.Channels, context.Bits, context.BufferSamples);
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
    char pos[16];
    auGetDeviceVendor(devId, vendor, sizeof(vendor));
    auGetDeviceModel(devId, model, sizeof(model));
    sprintf(title, "%s - %s (on %s %s)", "SndPlay", argv[1], vendor, model);

    context.Buffer.Pointer = malloc(frameSize);
    int bufferCount = auGetBufferCount(devId);

    wmInitialize();
    struct wmWindow *wnd = wmCreateWindow(200, 200, 400, 150, title, 1);
    wmShowWindow(wnd);

    struct uiControl *ctrlDisplay = uiControlCreate(wnd->RootControl, 0, NULL, 0, 0, wnd->ClientRectangle.Width, wnd->ClientRectangle.Height - 24, NULL, NULL);
    uiControlSetContext(ctrlDisplay, &context);
    uiControlSetOnPaint(ctrlDisplay, ctrlDisplayOnPaint);

    struct uiLabel *lblPosition = uiLabelCreate(wnd->RootControl, 0, wnd->ClientRectangle.Height - 24, 48, 24, "00:00", NULL, NULL);
    uiLabelSetHorizontalCentering(lblPosition, 1);

    struct uiButton *btnPlay = uiButtonCreate(wnd->RootControl, 48, wnd->ClientRectangle.Height - 24, 24, 24, ">", NULL, NULL);
    struct uiButton *btnPause = uiButtonCreate(wnd->RootControl, 72, wnd->ClientRectangle.Height - 24, 24, 24, "||", NULL, NULL);
    struct uiButton *btnStop = uiButtonCreate(wnd->RootControl, 96, wnd->ClientRectangle.Height - 24, 24, 24, "\xFF", NULL, NULL);
    struct uiSlider *sldPosition = uiSliderCreate(wnd->RootControl, 120, wnd->ClientRectangle.Height - 24, wnd->ClientRectangle.Width - 120, 24, 1, 0, context.TotalSamples, 0);

    uiControlRedraw(wnd->RootControl);

    struct wmEvent event;
    context.Frame = 0;
    res = 0;
    int playing = 1;
    int oldPos = 0;
    for(;; ++context.Frame)
    {
        int quit = 0;
        while((playing ? wmPeekEvent(wnd, &event, 1) : wmGetEvent(wnd, &event)) > 0)
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
                else if(event.Keyboard.Key == VK_LEFT)
                {
                    context.Position -= context.Rate * 5;
                    if(context.Position < 0) context.Position = 0;
                }
                else if(event.Keyboard.Key == VK_RIGHT)
                {
                    context.Position += context.Rate * 5;
                    if(context.Position > context.TotalSamples) context.Position = context.TotalSamples;
                }
            }
        }
        if(quit) break;

        if(!playing) continue;

        if(context.Position != oldPos)
            fseek(f, 44 + context.Position * context.BytesPerSample, SEEK_SET);

        int samplesToRead = context.TotalSamples - context.Position;
        if(samplesToRead > context.BufferSamples) samplesToRead = context.BufferSamples;
        int bytesToRead = context.BytesPerSample * samplesToRead;

        res = fread(context.Buffer.Pointer, 1, bytesToRead, f);
        if(res < 0)
        {
            printf("Read error\n");
            break;
        }
        else if(res != frameSize)
        {
            if(context.Bits == 8) memset(context.Buffer.Bytes + res, 128, frameSize - res);
            else memset(context.Buffer.Bytes + res, 0, frameSize - res);
        }

        if(auWriteDevice(devId, context.Buffer.Pointer))
        {
            printf("Playback error\n");
            break;
        }
        context.Position += res / context.BytesPerSample;
        oldPos = context.Position;

        uiControlRedraw(ctrlDisplay);
        wmUpdateWindow(wnd);

        if(context.Frame == (bufferCount - 1))
            auStartPlayback(devId);
        if(res != frameSize)
        {
            auStopPlayback(devId);
            playing = 0;
        }
        int seconds = context.Position / context.Rate;
        sprintf(pos, "%.2d:%.2d", seconds / 60, seconds % 60);
        uiControlSetText((struct uiControl *)lblPosition, pos);
        uiControlRedraw((struct uiControl *)lblPosition);
        uiSliderSetValue(sldPosition, context.Position);
        uiControlRedraw((struct uiControl *)sldPosition);
    }
    auStopPlayback(devId);
    wmDeleteWindow(wnd);
    wmCleanup();
    free(context.Buffer.Pointer);
    auCloseDevice(devId);
    fclose(f);
    return res;
}

