#include <cpu.h>
#include <errno.h>
#include <irqs.h>
#include <new.h>
#include <paging.h>
#include <pci.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysdefs.h>
#include <time.h>
#include <uhcicontroller.h>

#define UHCI_MAX_PORTS  127 // usb spec says 255 but that makes no sense since there can be
                            // up to 127 usb devices on a single controller

#define USBCMD          0x00
#define USBSTS          0x02
#define USBINTR         0x04
#define FRNUM           0x06
#define FRBASEADD       0x08
#define SOFMOD          0x0C
#define PORTSC          0x10

#define CMD_RS          0x0001
#define CMD_HCRESET     0x0002

#define STS_HCHALTED    0x0020

#define INTR_TO_CRC     0x0001
#define INTR_RESUME     0x0002
#define INTR_IOC        0x0004
#define INTR_SP         0x0008
#define INTR_ALL        (INTR_TO_CRC | INTR_RESUME | INTR_IOC | INTR_SP)

struct QueueHead
{
    uint32_t HLP; // Queue Head Link Pointer
    uint32_t ELP; // Queue Element Link Pointer
};

struct TransferDerscriptor
{
    uint32_t LP; // Link Pointer
    uint32_t Flags;
    uint32_t Addr;
    uint32_t Buffer;
};

struct ETD  // transfer descriptor with some extra kernel data
{           // must be 16 bytes aligned
    TransferDerscriptor TD;
    Semaphore *Sem;
    int Flags;
    int Padding[2];
};

bool UHCIController::interrupt(Ints::State *state, void *context)
{
    UHCIController *uhci = (UHCIController *)context;
    uint16_t sts = uhci->readw(USBSTS);
    if(!(sts & 0x03)) return false;    

    int16_t frNum = (uhci->readw(FRNUM) - 1) & 0x3FF;
    printf("[uhci] interrupt (%#.4x %#.4x %#.4x)\n", uhci->base, sts, frNum);

    uhci->frameList[frNum] |= 1;

    if(sts & STS_HCHALTED)
    {
        printf("      halted\n");
    }
    else
    {

    }

    uhci->writew(USBSTS, 0xFFFF); // clear status flags
    return true;
}

UHCIController::UHCIController(uint16_t base, uint8_t irq) :
    base(base),
    irq(irq),
    interruptHandler({ nullptr, interrupt, this }),
    frameList(new(PAGE_SIZE) dword[1024]),
    frameListPhAddr(Paging::GetPhysicalAddress(Paging::GetAddressSpace(), (uintptr_t)frameList)),
    ETDs(new(16) ETD[1024])
{
    // TODO: add legacy kb and mouse disable code here

    // detect port count (uhci spec mentions that there can be more than usual 2)
    for(int i = 0; i < UHCI_MAX_PORTS; ++i)
    {
        uint16_t portsc = readw(PORTSC + i * 2);
        if(!(portsc & 0x0080) || (portsc & 0xEC00))
            break;
        ++portCount;
    }
    //printf("       port count: %d\n", portCount);

    IRQs::RegisterHandler(irq, &interruptHandler);
    IRQs::Enable(irq);

    reset();
    writed(FRBASEADD, frameListPhAddr); // set frame list address
    writew(USBSTS, 0xFFFF); // clear all possible status bits
    enableInterrupts();

    // initialize frame list
    for(int i = 0; i < 1024; ++i)
        frameList[i] = 1;
}

uint16_t UHCIController::readw(uint16_t reg)
{
    return _inw(base + reg);
}

void UHCIController::writew(uint16_t reg, uint16_t val)
{
    _outw(base + reg, val);
}

void UHCIController::writed(uint16_t reg, uint32_t val)
{
    _outl(base + reg, val);
}

void UHCIController::setw(uint16_t reg, uint16_t mask)
{
    writew(reg, readw(reg) | mask);
}

void UHCIController::clrw(uint16_t reg, uint16_t mask)
{
    writew(reg, readw(reg) & ~mask);
}

void UHCIController::reset()
{
    setw(USBCMD, CMD_HCRESET);
    Time::Sleep(10, false);
    clrw(USBCMD, CMD_HCRESET);
}

void UHCIController::enableInterrupts()
{
    setw(USBINTR, INTR_ALL);
}

void UHCIController::stop()
{
    clrw(USBCMD, CMD_RS);
    while(!(readw(USBSTS) & STS_HCHALTED))
        Time::Sleep(1, false);
}

void UHCIController::start()
{
    setw(USBCMD, CMD_RS);
}

ETD *UHCIController::allocETD()
{
    for(int i = 0; i < 1024; ++i)
    {
        ETD *etd = ETDs + i;
        if(!(etd->Flags & 1))
            return etd;
    }
    return nullptr;
}

void UHCIController::freeETD(ETD *etd)
{
    if(etd) etd->Flags &= ~1;
}

UHCIController::~UHCIController()
{
    // global reset
    _outw(base + USBCMD, _inw(base + USBCMD) | 0x0002);
    Time::Sleep(10, false);
    _outw(base + USBCMD, _inw(base + USBCMD) & ~0x0002);

    IRQs::TryDisable(irq);
    IRQs::UnRegisterHandler(irq, &interruptHandler);

    delete[] ETDs;
    delete[] frameList;
}

int UHCIController::Transfer(void *buffer, int n, uint8_t pid, uint8_t address, uint8_t endpoint)
{
    return -ENOSYS;
}

void UHCIController::Initialize()
{
    if(!PCI::Lock->Wait(0, false, false))
        return;
    for(PCI::Device *pciDev : *PCI::Devices)
    {
        if(pciDev->Class != 0x0C || pciDev->SubClass != 0x03 || pciDev->ProgIF != 0x00)
            continue;
        printf("[uhci] Found UHCI controller at PCI:%d.%d.%d\n",
               PCI_ADDR_BUS(pciDev->Address),
               PCI_ADDR_DEV(pciDev->Address),
               PCI_ADDR_FUNC(pciDev->Address));

        // enable PCI BusMaster
        PCI::WriteConfigWord(pciDev->Address | 0x04, PCI::ReadConfigWord(pciDev->Address | 0x04) | 0x0004);

        PCI::Config cfg;
        PCI::ReadConfigData(&cfg, pciDev->Address);

        uint16_t base = cfg.Header.Default.BAR[4] & ~0x03;
        uint8_t irq = cfg.Header.Default.InterruptLine;
        printf("       base: %#.4x irq: %d id: %d\n", base, irq, USBController::Add(new UHCIController(base, irq)));
    }
    PCI::Lock->Signal(nullptr);
}

void UHCIController::Cleanup()
{

}

