#ifndef CMI8738_H
#define CMI8738_H

#include <ints.h>
#include <types.h>

class CMI8738
{
    static bool interrupt(Ints::State *state, void *context);

    uint16_t base;
    uint8_t irq;
public:
    static void Initialize();
    static void Cleanup();
};

#endif // CMI8738_H
