#ifndef ES1371_H
#define ES1371_H

#include <audiodevice.h>
#include <ints.h>
#include <types.h>

class Semaphore;

class ES1371 : public AudioDevice
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
    virtual const char *GetVendor();
    virtual const char *GetModel();
    virtual int Open(int rate, int channels, int bits, int samples);
    virtual int GetFrameSize();
    virtual int Start();
    virtual int Stop();
    virtual int Write(void *buffer);
    virtual void Close();
    ~ES1371();
};

#endif // ES1371_H
