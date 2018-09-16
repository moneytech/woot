#include <cmos.h>
#include <cpu.h>

byte CMOS::Read(byte reg)
{
    bool cs = cpuDisableInterrupts();
    _outb(0x70, (_inb(0x70) & 0x80) | (reg & 0x7F));
    byte val = _inb(0x71);
    cpuRestoreInterrupts(cs);
    return val;
}

void CMOS::Write(byte reg, byte val)
{
    bool cs = cpuDisableInterrupts();
    _outb(0x70, (_inb(0x70) & 0x80) | (reg & 0x7F));
    _outb(0x71, val);
    cpuRestoreInterrupts(cs);
}

bool CMOS::DisableNMI()
{
    bool cs = cpuDisableInterrupts();
    byte v = _inb(0x70);
    _outb(0x70, v & 0x7F);
    cpuRestoreInterrupts(cs);
}

void CMOS::RestoreNMI(bool state)
{
    bool cs = cpuDisableInterrupts();
    byte v = _inb(0x70) & 0x7F;
    _outb(0x70, v | (state ? 0x80 : 0x00));
    cpuRestoreInterrupts(cs);
}

uint CMOS::GetIRQ()
{
    return 8;
}

void CMOS::EnableTimer()
{
    bool cs = cpuDisableInterrupts();
    bool nmi = DisableNMI();
    Write(0x0B, Read(0x0B) | 0x40);
    RestoreNMI(nmi);
    cpuRestoreInterrupts(cs);
}

void CMOS::DisableTimer()
{
    bool cs = cpuDisableInterrupts();
    bool nmi = DisableNMI();
    Write(0x0B, Read(0x0B) & ~0x40);
    RestoreNMI(nmi);
    cpuRestoreInterrupts(cs);
}

int CMOS::GetTimerFrequency()
{
    bool cs = cpuDisableInterrupts();
    int rate = Read(0x0A) & 0x0F;
    cpuRestoreInterrupts(cs);
    return 32768 >> (rate - 1);
}

void CMOS::SetTimerDivider(int divider)
{
    if(divider < 3) divider = 3;
    else if(divider > 15) divider = 15;
    bool cs = cpuDisableInterrupts();
    bool nmi = DisableNMI();
    Write(0x0A, (Read(0x0A) & 0xF0) | divider);
    RestoreNMI(nmi);
    cpuRestoreInterrupts(cs);
}
