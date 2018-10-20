#ifndef PS2MOUSE_H
#define PS2MOUSE_H

#include <inputdevice.h>

class PS2Mouse : public InputDevice
{
    static bool isr(Ints::State *state, void *context);
    Ints::Handler handler;
    uint16_t dataPort, cmdPort;
    uint8_t irq;
    int dataPhase;
    byte data[3];
    int buttons;
    PS2Mouse(uint16_t data, uint16_t cmd, uint8_t irq);
    ~PS2Mouse();
public:
    static void Initialize();
    static void Cleanup();
};

#endif // PS2MOUSE_H
