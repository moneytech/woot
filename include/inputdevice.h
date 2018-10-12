#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <list.h>
#include <mutex.h>
#include <types.h>
#include <virtualkey.h>

class InputDevice
{
    static List<InputDevice *> devices;
    static Mutex listLock;
public:
    enum class Type
    {
        Unknown = 0,
        Keyboard,
        Mouse,
        GameController,
        Other
    };

    class Event
    {
    public:
        InputDevice *Device;    // using this requires a lock
        Type DeviceType;        // and using this, does not (must be the same as Device->DeviceType)
        union
        {
            struct
            {
                VirtualKey Key;
                bool Release;
            } Keyboard;
        };
        Event(InputDevice *dev, Type devType);
        Event(InputDevice *dev, VirtualKey key, bool release);
    };

    static void Initialize();
    static bool Lock();
    static bool Add(InputDevice *dev);
    static InputDevice *GetFirstByType(Type type);
    static bool Remove(InputDevice *dev);
    static void UnLock();
    static void Cleanup();

    Type DeviceType;
    char *Name;
    InputDevice(Type type, const char *name);
    virtual Event GetEvent(uint timeout);
    virtual ~InputDevice();
};

#endif // INPUTDEVICE_H
