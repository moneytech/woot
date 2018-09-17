#include <cpu.h>
#include <ints.h>
#include <irqs.h>

#define VECTOR_COUNT 256

Ints::Handler *Ints::Handlers[VECTOR_COUNT];

extern "C" void intsCommonHandler(Ints::State *state)
{
    Ints::CommonHandler(state);
}

void Ints::CommonHandler(Ints::State *state)
{
    int irq = state->InterruptNumber - IRQS_BASE;
    bool isIrq = irq >= 0 && irq < IRQS_COUNT;

    // handle spurious irqs
    if(isIrq)
    {
        if(IRQs::IsSpurious(irq))
        {
            IRQs::HandleSpurious(irq);
            return;
        }
    }

    bool handled = false;
    Handler *handler = Handlers[state->InterruptNumber];
    for(handled = false; !handled && handler && handler->Callback; handler = handler->Next)
        handled = handler->Callback(state, handler->Context);
    if(!handled)
    {
        _outsb("Unhandled interrupt/exception. System Halted!\r\n", 0xE9, 47);
        cpuSystemHalt(state->InterruptNumber);
    }

    if(isIrq) IRQs::SendEOI(irq);
}

void Ints::RegisterHandler(uint intNo, Ints::Handler *handler)
{
    if(intNo >= VECTOR_COUNT || !handler)
        return;
    bool ints = cpuDisableInterrupts();
    handler->Next = Handlers[intNo];
    Handlers[intNo] = handler;
    cpuRestoreInterrupts(ints);
}

void Ints::UnRegisterHandler(uint intNo, Ints::Handler *handler)
{
    if(intNo >= VECTOR_COUNT || !handler)
        return;
    bool ints = cpuDisableInterrupts();
    for(Handler *prev = 0, *h = Handlers[intNo]; h; prev = h, h = h->Next)
    {
        if(handler == h)
        {
            if(prev) prev->Next = h->Next;
            else Handlers[intNo] = h->Next;
        }
    }
    cpuRestoreInterrupts(ints);
}
