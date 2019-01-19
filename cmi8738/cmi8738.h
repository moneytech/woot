#ifndef CMI8738_H
#define CMI8738_H

#include <audiodevice.h>
#include <ints.h>
#include <types.h>

class Semaphore;

class CMI8738 : public AudioDevice
{
    static bool interrupt(Ints::State *state, void *context);

    uint16_t base;
    uint8_t irq;
    Ints::Handler interruptHandler;
    bool opened = false;

    Semaphore *bufSem;
    int samples;
    size_t bufferSize;
    byte *buffer;
    uintptr_t bufferPhAddr;
    uint32_t sf, fmt;
    int playedBuffer;
    bool last;

    void mixerWrite(byte idx, byte val);
    byte mixerRead(byte idx);
    void mixerReset();
    void chanReset(int ch);
    void bmDspReset();
    void fullReset();
    uint32_t getSFSel(int playbackRate);
    uint32_t getFmt(int playbackChannels, int playbackBits);
public:
    static void Initialize();
    static void Cleanup();

    CMI8738(uint16_t base, uint8_t irq);
    virtual const char *GetVendor();
    virtual const char *GetModel();
    virtual const MixerSetting *GetMixerSettings(int *count);
    virtual int SetMixerSetting(int setting, int value);
    virtual int Open(int rate, int channels, int bits, int samples);
    virtual int GetFrameSize();
    virtual int Start();
    virtual int Stop();
    virtual int Pause();
    virtual int Resume();
    virtual int Write(void *buffer);
    virtual void Close();
    ~CMI8738();
};

#endif // CMI8738_H
