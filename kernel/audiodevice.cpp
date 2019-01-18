#include <audiodevice.h>
#include <errno.h>

Sequencer<int> AudioDevice::ids(0);
List<AudioDevice *> AudioDevice::devices;
Mutex AudioDevice::lock;

int AudioDevice::Add(AudioDevice *device)
{
    int res = Lock();
    if(res) return res;
    devices.Append(device);
    UnLock();
    return 0;
}

int AudioDevice::Lock()
{
    return lock.Acquire(0) ? 0 : -EBUSY;
}

AudioDevice *AudioDevice::GetByID_nolock(int id)
{
    for(AudioDevice *device : devices)
    {
        if(device->ID == id)
            return device;
    }
    return nullptr;
}

AudioDevice *AudioDevice::GetByID(int id)
{
    if(Lock()) return nullptr;
    AudioDevice *res = GetByID_nolock(id);
    UnLock();
    return res;
}

void AudioDevice::UnLock()
{
    lock.Release();
}

void AudioDevice::Remove_nolock(int id)
{
    AudioDevice *device = GetByID_nolock(id);
    if(device) devices.Remove(device, nullptr, false);
}

void AudioDevice::Remove(int id)
{
    if(Lock()) return;
    Remove_nolock(id);
    UnLock();
}

AudioDevice::AudioDevice() :
    ID(ids.GetNext())
{
}

const char *AudioDevice::GetVendor()
{
    return "unknown";
}

const char *AudioDevice::GetModel()
{
    return "unknown";
}

int AudioDevice::Open(int rate, int channels, int bits, int samples)
{
    return -ENOSYS;
}

int AudioDevice::Start()
{
    return -ENOSYS;
}

int  AudioDevice::Stop()
{
    return -ENOSYS;
}

int AudioDevice::Pause()
{
    return -ENOSYS;
}

int AudioDevice::Resume()
{
    return -ENOSYS;
}

void AudioDevice::Close()
{
}

AudioDevice::~AudioDevice()
{
}

