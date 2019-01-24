#ifndef ES1371_H
#define ES1371_H

#include <ac97.h>
#include <audiodevice.h>
#include <ints.h>
#include <types.h>

class Semaphore;
class Thread;

class ES1371 : public AudioDevice, public AC97
{
    static bool interrupt(Ints::State *state, void *context);

    uint16_t base;
    uint8_t irq;

    Ints::Handler interruptHandler;
    bool opened = false;
    Thread *owner = nullptr;


    Semaphore *bufSem;
    int samples;
    size_t bufferSize;
    byte *buffer;
    uintptr_t bufferPhAddr;
    int playedBuffer;
    int writtenBuffer;

    void codecReset();
    int codecWrite(uint8_t reg, uint16_t val);
    int codecRead(uint8_t reg);

    int srcReset();
    int srcWrite(uint8_t reg, uint16_t val);
    int srcRead(uint8_t reg);
    int srcSetRate(int base, int rate);
public:
    static void Initialize();
    static void Cleanup();

    ES1371(uint16_t base, uint8_t irq);

    // AudioDevice
    virtual const char *GetVendor();
    virtual const char *GetModel();
    virtual const MixerSetting *GetMixerSettings(int *count);
    virtual int SetMixerSetting(int setting, int value);
    virtual int GetMixerSetting(int setting);
    virtual int GetBufferCount();
    virtual int Open(int rate, int channels, int bits, int samples);
    virtual int GetFrameSize();
    virtual int Start();
    virtual int Stop();
    virtual int Write(void *buffer);
    virtual void Close();

    // AC97
    virtual int RegisterWrite(uint8_t reg, uint16_t val);
    virtual int RegisterRead(uint8_t reg);

    ~ES1371();
};

#endif // ES1371_H
