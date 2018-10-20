#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <list.h>
#include <mutex.h>
#include <queue.h>
#include <semaphore.h>
#include <types.h>
#include <virtualkey.h>

#define INPUT_MAX_MOUSE_AXES 4

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
            struct
            {
                int Movement[INPUT_MAX_MOUSE_AXES];
                int ButtonsPressed;
                int ButtonsReleased;
                int ButtonsHeld;
            } Mouse;
        };
        Event();
        Event(InputDevice *dev, Type devType); // general event
        Event(InputDevice *dev, VirtualKey key, bool release); // keyboard event
        Event(InputDevice *dev, int *movement, int pressed, int released, int held); // mouse event
    };
protected:
    static Semaphore eventSemaphore;
    static Queue<Event> eventQueue;
public:
    static void Initialize();
    static bool Lock();
    static bool Add(InputDevice *dev);
    static Event GetEvent(uint timeout);
    static InputDevice *GetFirstByType(Type type);
    static bool Remove(InputDevice *dev);
    static void UnLock();
    static void Cleanup();

    Type DeviceType;
    char *Name;
    InputDevice(Type type, const char *name);
    virtual ~InputDevice();
};

#endif // INPUTDEVICE_H
