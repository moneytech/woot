#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <woot/audio.h>

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
    int samples = 4096;

    fseek(f, 6, SEEK_CUR);
    fread(&channels, 2, 1, f);
    fread(&rate, 4, 1, f);
    fseek(f, 6, SEEK_CUR);
    fread(&bits, 2, 1, f);
    fseek(f, 6, SEEK_CUR);

    int devId = 0;
    if(argc >= 3) devId = strtoul(argv[2], NULL, 0);

    printf("Opening device %d: %d Hz %d channel(s) %d bits per sample\n", devId, rate, channels, bits);

    res = auOpenDevice(devId, rate, channels, bits, 4096);
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

    uint8_t *buffer = (uint8_t *)malloc(frameSize);
    int bufferCount = auGetBufferCount(devId);

    res = 0;
    for(int i = 0;; ++i)
    {
        res = fread(buffer, 1, frameSize, f);
        if(res < 0)
        {
            printf("Read error\n");
            break;
        }
        else if(res != frameSize)
        {
            memset(buffer + res, 0, frameSize - res);
            res = 0;
        }
        auWriteDevice(devId, buffer);
        if(i == (bufferCount - 1))
            auStartPlayback(devId);
        if(res != frameSize) break;
    }
    auStopPlayback(devId);
    free(buffer);
    auCloseDevice(devId);
    fclose(f);
    return res;
}

