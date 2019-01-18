#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <list.h>
#include <mutex.h>
#include <sequencer.h>
#include <types.h>

class AudioDevice
{
    static Sequencer<int> ids;
    static List<AudioDevice *> devices;
    static Mutex lock;
public:
    int ID;

    static int Add(AudioDevice *device);
    static int Lock();
    static AudioDevice *GetByID_nolock(int id);
    static AudioDevice *GetByID(int id);
    static void UnLock();
    static void Remove_nolock(int id);
    static void Remove(int id);

    AudioDevice();
    virtual const char *GetVendor();
    virtual const char *GetModel();
    virtual int Open(int rate, int channels, int bits, int samples);
    virtual int Start();
    virtual int Stop();
    virtual int Pause();
    virtual int Resume();
    virtual void Close();
    ~AudioDevice();
};

#endif // AUDIODEVICE_H
