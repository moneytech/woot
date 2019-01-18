#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <list.h>
#include <mutex.h>
#include <types.h>


class AudioDevice
{
    static List<AudioDevice *> devices;
    static Mutex lock;
public:

};

#endif // AUDIODEVICE_H
