#include <cpu.h>
#include <gdt.h>
#include <idt.h>
#include <ints.h>
#include <irqs.h>

bool divByZero(Ints::State *state, void *context)
{
    _outsb("Division by zero\r\n", 0xE9, 18);
    return true;
}
Ints::Handler divByZeroHandler = { nullptr, divByZero, nullptr };

bool kbdTest(Ints::State *state, void *context)
{
    _inb(0x60);
    _outb(0xE9, '.');
    IRQs::SendEOI(1);
    return true;
}
Ints::Handler kbdTestHandler = { nullptr, kbdTest, nullptr };

extern "C" int kmain(void *mbootInfo)
{
    GDT::Initialize();
    IDT::Initialize();
    IRQs::Initialize();
    cpuEnableInterrupts();

    unsigned short *video = (unsigned short *)0xC00B8000;
    video[0] = 0x1F00 | 'X';

    Ints::RegisterHandler(0u, &divByZeroHandler);
    asm("int $0");

    IRQs::RegisterHandler(1, &kbdTestHandler);
    IRQs::Enable(1);

    for(;;) cpuWaitForInterrupt(0);

    return 0xD007D007;
}
