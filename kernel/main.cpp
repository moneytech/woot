#include <cpu.h>
#include <gdt.h>
#include <idt.h>
#include <ints.h>

bool divByZero(Ints::State *state, void *context)
{
    _outsb("Division by zero\r\n", 0xE9, 18);
    return false;
}
Ints::Handler divByZeroHandler = { nullptr, divByZero, nullptr };

extern "C" int kmain(void *mbootInfo)
{
    GDT::Initialize();
    IDT::Initialize();

    unsigned short *video = (unsigned short *)0xC00B8000;
    video[0] = 0x1F00 | 'X';

    Ints::RegisterHandler(0, &divByZeroHandler);
    asm("int $0");

    return 0xD007D007;
}
