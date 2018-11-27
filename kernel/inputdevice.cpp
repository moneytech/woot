#include <cpu.h>
#include <inputdevice.h>
#include <stdlib.h>
#include <string.h>

List<InputDevice *> InputDevice::devices;
Mutex InputDevice::listLock("inputList");
Semaphore InputDevice::eventSemaphore(0);
Queue<InputDevice::Event> InputDevice::eventQueue(256);

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

InputDevice::Event InputDevice::GetEvent(uint timeout)
{
    bool ints = cpuAreInterruptsEnabled();
    if(!eventSemaphore.Wait(timeout, false, true))
        return Event();
    Event event = eventQueue.Read(nullptr);
    cpuRestoreInterrupts(ints);
    return event;
}

InputDevice::Event InputDevice::PeekEvent()
{
    bool ints = cpuDisableInterrupts();
    Event event = eventQueue.Peek();
    cpuRestoreInterrupts(ints);
    return event;
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

InputDevice::~InputDevice()
{
    if(Name) free(Name);
}

InputDevice::Event::Event() :
    Device(nullptr), DeviceType(Type::Unknown)
{
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

InputDevice::Event::Event(InputDevice *dev, int *movement, int pressed, int released, int held) :
    Device(dev), DeviceType(Type::Mouse),
    Mouse({ { }, pressed, released, held })
{
    memcpy(Mouse.Movement, movement, sizeof(int) * INPUT_MAX_MOUSE_AXES);
}
