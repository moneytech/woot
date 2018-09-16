#ifndef IRQS_H
#define IRQS_H

#define IRQS_BASE  32
#define IRQS_COUNT 16

#include <ints.h>
#include <types.h>

class IRQs
{
public:
    static void Initialize();
    static void Enable(uint irq);
    static void Disable(uint irq);
    static bool IsEnabled(uint irq);
    static void SendEOI(uint irq);
    static bool IsSpurious(uint irq);
    static void HandleSpurious(uint irq);
    static void RegisterHandler(uint irq, Ints::Handler *handler);
    static void UnRegisterHandler(uint irq, Ints::Handler *handler);
};

#endif // IRQS_H
