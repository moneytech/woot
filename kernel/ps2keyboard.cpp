#include <cpu.h>
#include <errno.h>
#include <ints.h>
#include <irqs.h>
#include <ps2keyboard.h>
#include <time.h>

VirtualKey scancodeTable[] =
{
    // 0x00
    VirtualKey::None, VirtualKey::Escape, VirtualKey::Key1, VirtualKey::Key2,
    VirtualKey::Key3, VirtualKey::Key4, VirtualKey::Key5, VirtualKey::Key6,
    VirtualKey::Key7, VirtualKey::Key8, VirtualKey::Key9, VirtualKey::Key0,
    VirtualKey::OEMMinus, VirtualKey::OEMPlus, VirtualKey::Back, VirtualKey::Tab,
    // 0x10
    VirtualKey::KeyQ, VirtualKey::KeyW, VirtualKey::KeyE, VirtualKey::KeyR,
    VirtualKey::KeyT, VirtualKey::KeyY, VirtualKey::KeyU, VirtualKey::KeyI,
    VirtualKey::KeyO, VirtualKey::KeyP, VirtualKey::OEM4, VirtualKey::OEM6,
    VirtualKey::Return, VirtualKey::LControl, VirtualKey::KeyA, VirtualKey::KeyS,
    // 0x20
    VirtualKey::KeyD, VirtualKey::KeyF, VirtualKey::KeyG, VirtualKey::KeyH,
    VirtualKey::KeyJ, VirtualKey::KeyK, VirtualKey::KeyL, VirtualKey::OEM1,
    VirtualKey::OEM7, VirtualKey::OEM3, VirtualKey::LShift, VirtualKey::OEM5,
    VirtualKey::KeyZ, VirtualKey::KeyX, VirtualKey::KeyC, VirtualKey::KeyV,
    // 0x30
    VirtualKey::KeyB, VirtualKey::KeyN, VirtualKey::KeyM, VirtualKey::OEMComma,
    VirtualKey::OEMPeriod, VirtualKey::OEM2, VirtualKey::RShift, VirtualKey::Multiply,
    VirtualKey::LMenu, VirtualKey::Space, VirtualKey::Capital, VirtualKey::F1,
    VirtualKey::F2, VirtualKey::F3, VirtualKey::F4, VirtualKey::F5,
    // 0x40
    VirtualKey::F6, VirtualKey::F7, VirtualKey::F8, VirtualKey::F9,
    VirtualKey::F10, VirtualKey::NumLock, VirtualKey::Scroll, VirtualKey::NumPad7,
    VirtualKey::NumPad8, VirtualKey::NumPad9, VirtualKey::Subtract, VirtualKey::NumPad4,
    VirtualKey::NumPad5, VirtualKey::NumPad6, VirtualKey::Add, VirtualKey::NumPad1,
    // 0x50
    VirtualKey::NumPad2, VirtualKey::NumPad3, VirtualKey::NumPad0, VirtualKey::Decimal,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::F11,
    VirtualKey::F12, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x60
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x70
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x80
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x90
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xA0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xB0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xC0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::Home,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xD0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xE0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xF0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
};

VirtualKey scancodeTableEx[] =
{
    // 0x00
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x10
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::Return, VirtualKey::RControl, VirtualKey::None, VirtualKey::None,
    // 0x20
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x30
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::Divide, VirtualKey::None, VirtualKey::None,
    VirtualKey::RMenu, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x40
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::Home,
    VirtualKey::Up, VirtualKey::Prior, VirtualKey::None, VirtualKey::Left,
    VirtualKey::None, VirtualKey::Right, VirtualKey::None, VirtualKey::End,
    // 0x50
    VirtualKey::Down, VirtualKey::Next, VirtualKey::Insert, VirtualKey::Delete,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::LWin,
    VirtualKey::RWin, VirtualKey::Apps, VirtualKey::None, VirtualKey::None,
    // 0x60
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x70
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x80
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x90
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xA0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xB0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xC0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::Home,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xD0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xE0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xF0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
};

bool PS2Keyboard::isr(Ints::State *state, void *context)
{
    PS2Keyboard *kbd = (PS2Keyboard *)context;
    byte data = _inb(kbd->dataPort);
    if(data == 0xE0)
    {
        kbd->ex = true;
        return true;
    }
    bool release = data & 0x80;
    data &= 0x7F;
    eventQueue.Write(Event(kbd, kbd->ex ? scancodeTableEx[data] : scancodeTable[data], release));
    kbd->ex = false;
    eventSemaphore.Signal(state);
    return true;
}

PS2Keyboard::PS2Keyboard(uint16_t data, uint16_t cmd, uint8_t irq) :
    InputDevice(Type::Keyboard, "PS/2 Keyboard"),
    handler { nullptr, isr, this },
    dataPort(data), cmdPort(cmd), irq(irq), ex(false)
{
    IRQs::RegisterHandler(irq, &handler);
    IRQs::Enable(irq);

    // set scancode set
    DeviceWrite(false, 0xF0);
    DeviceWrite(false, 2);
    DeviceRead();
}

PS2Keyboard::~PS2Keyboard()
{
    bool ints = cpuDisableInterrupts();
    IRQs::UnRegisterHandler(irq, &handler);
    IRQs::TryDisable(irq);
    cpuRestoreInterrupts(ints);
}

bool PS2Keyboard::DualPort = false;

void PS2Keyboard::Initialize()
{
    // do a couple of dummy reads
    for(int i = 0; i < 20; ++i)
        _inb(0x60);

    // do another dummy read
    _inb(0x60);

    // set new configuration byte
    ControllerCommand(0x60, false, true, 0x00);

    // one more dummy read
    _inb(0x60);

    // disable second port
    ControllerCommand(0xA7, false, false, 0);

    // check if second port clock reacted do disable command
    DualPort = ControllerCommand(0x20, true, false, 0) & 0x20;

    // reenable interrupts and second if present
    ControllerCommand(0x60, false, true, DualPort ? 0x43 : 0x63);

    PS2Keyboard *kbd = new PS2Keyboard(0x60, 0x64, 1);
    Add(kbd);
}

int PS2Keyboard::ControllerCommand(uint8_t cmd, bool resp, bool arg, uint8_t argv)
{
    // send command
    _outb(0x64, cmd);

    if(arg)
    {   // send argument if needed
        int retry = 100;
        while(_inb(0x64) & 0x02 && --retry)
            Time::Sleep(1, false);
        if(!retry) return -EBUSY;
        _outb(0x60, argv);
    }

    if(resp)
    {   // read response if applicable
        int retry = 100;
        while(!(_inb(0x64) & 0x01) && --retry)
            Time::Sleep(1, false);
        if(!retry) return -EBUSY;
        return _inb(0x60);
    }

    return 0;
}

int PS2Keyboard::DeviceWrite(bool second, uint8_t val)
{
    if(second) _outb(0x64, 0xD4);
    int retry = 100;
    while(_inb(0x64) & 0x02 && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    _outb(0x60, val);
    return 0;
}

int PS2Keyboard::DeviceRead()
{
    int retry = 100;
    while(!(_inb(0x64) & 0x01) && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    return _inb(0x60);
}

void PS2Keyboard::Cleanup()
{
}
