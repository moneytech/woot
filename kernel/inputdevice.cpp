#include <inputdevice.h>
#include <stdlib.h>
#include <string.h>

List<InputDevice *> InputDevice::devices;
Mutex InputDevice::listLock;

void InputDevice::Initialize()
{
}

bool InputDevice::Lock()
{
    return listLock.Acquire(0, false);
}

bool InputDevice::Add(InputDevice *dev)
{
    if(!Lock()) return false;
    devices.Append(dev);
    UnLock();
    return true;
}

InputDevice *InputDevice::GetFirstByType(InputDevice::Type type)
{
    if(!Lock()) return nullptr;
    for(InputDevice *dev : devices)
    {
        if(dev->DeviceType == type)
        {
            UnLock();
            return dev;
        }
    }
    UnLock();
    return nullptr;
}

bool InputDevice::Remove(InputDevice *dev)
{
    if(!Lock()) return false;
    devices.Remove(dev, nullptr, false);
    UnLock();
    return true;
}

void InputDevice::UnLock()
{
    listLock.Release();
}

void InputDevice::Cleanup()
{
    for(InputDevice *dev : devices)
        delete dev;
}

InputDevice::InputDevice(Type type, const char *name) :
    DeviceType(type),
    Name(strdup(name))
{
}

InputDevice::Event InputDevice::GetEvent(uint timeout)
{
    return Event(this, Type::Unknown);
}

InputDevice::~InputDevice()
{
    if(Name) free(Name);
}

InputDevice::Event::Event(InputDevice *dev, Type devType) :
    Device(dev), DeviceType(devType)
{
}

InputDevice::Event::Event(InputDevice *dev, VirtualKey key, bool release) :
    Device(dev), DeviceType(Type::Keyboard),
    Keyboard({ key, release })
{
}
