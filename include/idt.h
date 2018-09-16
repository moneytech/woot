#ifndef IDT_H
#define IDT_H

#include <types.h>

#pragma pack(push, 1)

struct InterruptGate
{
    uint16_t Offset15_0;
    uint16_t Selector;
    uint8_t Zero;
    uint8_t Type : 4;
    uint8_t S : 1;
    uint8_t DPL : 2;
    uint8_t P : 1;
    uint16_t Offset31_16;
};

typedef InterruptGate TrapGate;
typedef InterruptGate CallGate;

struct IDTEntry
{
    union
    {
        uint32_t Value[2];
        InterruptGate Interrupt;
        TrapGate Trap;
        CallGate Call;
    };
};

struct IDTDescriptor
{
    uint16_t Limit;
    IDTEntry *Entries;
};

#pragma pack(pop)

class IDT
{
    static IDTDescriptor Descriptor;
    static IDTEntry Entries[256];
public:
    static void Initialize();
    static void SetInterruptEntry(uint8_t i, uint16_t sel, uintptr_t offset, uint8_t type);
};

#endif // IDT_H
