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
    Semaphore dataSem;
    Ints::Handler handler;
    uint16_t dataPort, cmdPort;
    uint8_t irq;
    Queue<byte> dataQueue;
    bool ex;
    PS2Keyboard(uint16_t data, uint16_t cmd, uint8_t irq);
    ~PS2Keyboard();
public:
    static void Initialize();
    static void Cleanup();

    virtual Event GetEvent(uint timeout);
};

#endif // PS2KEYBOARD_H