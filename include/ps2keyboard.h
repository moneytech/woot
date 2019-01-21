#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H

#include <ints.h>
#include <inputdevice.h>
#include <queue.h>
#include <semaphore.h>
#include <types.h>

class PS2Keyboard : public InputDevice
{
    static bool isr(Ints::State *state, void *context);
    Ints::Handler handler;
    uint16_t dataPort, cmdPort;
    uint8_t irq;
    bool ex;
    PS2Keyboard(uint16_t data, uint16_t cmd, uint8_t irq);
    ~PS2Keyboard();
public:
    static bool DualPort;

    static void Initialize();
    static int ControllerCommand(uint8_t cmd, bool resp, bool arg, uint8_t argv);
    static int DeviceWrite(bool second, uint8_t val);
    static int DeviceRead();
    static void Cleanup();

};

#endif // PS2KEYBOARD_H
