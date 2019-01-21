#include <cpu.h>
#include <ints.h>
#include <irqs.h>
#include <ps2keyboard.h>
#include <ps2mouse.h>
#include <time.h>

bool PS2Mouse::isr(Ints::State *state, void *context)
{
    PS2Mouse *mouse = (PS2Mouse *)context;
    byte b = _inb(mouse->dataPort);
    if(!mouse->dataPhase)
    {
        if(b == 0xFA || b == 0xFE)
            return true;
        if(!(b & 0x08))
            return true; // ignore this byte (try to resynchronize)
    }
    mouse->data[mouse->dataPhase++] = b;
    if(mouse->dataPhase < 3)
        return true;
    mouse->dataPhase = 0;

    int newButtons = mouse->data[0] & 0x03;
    int dx = mouse->data[1] - ((mouse->data[0] << 4) & 0x100);
    int dy = -(mouse->data[2] - ((mouse->data[0] << 3) & 0x100));
    int pressed = (mouse->buttons ^ newButtons) & newButtons;
    int released = (mouse->buttons ^ newButtons) & mouse->buttons;
    mouse->buttons = newButtons;

    int movement[INPUT_MAX_MOUSE_AXES] = { dx, dy };

    eventQueue.Write(Event(mouse, movement, pressed, released, mouse->buttons));
    eventSemaphore.Signal(state);
    return true;
}

PS2Mouse::PS2Mouse(uint16_t data, uint16_t cmd, uint8_t irq) :
    InputDevice(Type::Mouse, "PS/2 Mouse"),
    handler { nullptr, isr, this },
    dataPort(data), cmdPort(cmd), irq(irq),
    dataPhase(0)
{
    PS2Keyboard::DeviceWrite(true, 0xF4);
    IRQs::SendEOI(irq);

    IRQs::RegisterHandler(irq, &handler);
    IRQs::Enable(irq);
}

PS2Mouse::~PS2Mouse()
{
    bool ints = cpuDisableInterrupts();
    IRQs::UnRegisterHandler(irq, &handler);
    IRQs::TryDisable(irq);
    cpuRestoreInterrupts(ints);
}

void PS2Mouse::Initialize()
{
    // TODO: add actual detection and initialization code
    PS2Mouse *mouse = new PS2Mouse(0x60, 0x64, 12);
    InputDevice::Add(mouse);
}

void PS2Mouse::Cleanup()
{
}
