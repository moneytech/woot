#include <cpu.h>
#include <errno.h>
#include <irqs.h>
#include <new.h>
#include <paging.h>
#include <pci.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdefs.h>
#include <time.h>
#include <uhcicontroller.h>
#include <uhcidevice.h>
#include <usbdevice.h>

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
#define STS_PROC_ERR    0x0010
#define STS_SYS_ERR     0x0008
#define STS_RES_DET     0x0004
#define STS_USB_ERROR   0x0002
#define STS_USBINT      0x0001

#define INTR_TO_CRC     0x0001
#define INTR_RESUME     0x0002
#define INTR_IOC        0x0004
#define INTR_SP         0x0008
#define INTR_ALL        (INTR_TO_CRC | INTR_RESUME | INTR_IOC | INTR_SP)

struct QueueHead
{
    uint32_t HLP; // Queue Head Link Pointer
    uint32_t ELP; // Queue Element Link Pointer

    QueueHead *Self; // Virtual address of this queue head
    QueueHead *Next; // Virtual address of next queue head
    TransferDescriptor *Child; // Virtual address of first transfer descriptor
    Semaphore *Waiter; // Semaphore to signal on completion
    uint32_t Padding[2];

    QueueHead() :
        HLP(1), ELP(1),
        Self(this), Next(nullptr),
        Child(nullptr), Waiter(nullptr)
    {
    }
};

struct TransferDescriptor
{
    uint32_t LP; // Link Pointer
    uint32_t CtrlSts;
    uint32_t Token;
    uint32_t Buffer;

    TransferDescriptor *Self;   // Virtual address of this transfer descriptor
    TransferDescriptor *Next;   // Virtual address of next transfer descriptor
    uint32_t Padding[2];

    TransferDescriptor() :
        LP(1), CtrlSts(0), Token(0), Buffer(0),
        Self(this), Padding { 0, 0 }
    {
    }

    TransferDescriptor(uintptr_t link, bool vf, bool t, bool ioc, bool iso, bool ls, uint8_t pid, uint8_t addr, uint8_t endpt, bool d, size_t maxLen, uintptr_t buffer) :
        LP((link & 0xFFFFFFF0) | (t ? 1 : 0) | (vf ? 4 : 0)),
        CtrlSts((ioc ? 1 << 24 : 0) | (iso ? 1 << 25 : 0) | (ls ? 1 << 26 : 0) | 1 << 23),
        Token(((maxLen - 1) & 0x07FF) << 21 | (d ? 1 << 19 : 0) | (endpt & 0x0F) << 15 | (addr & 0x7F) << 8 | pid),
        Buffer(buffer), Self(this), Padding { 0, 0 }
    {
    }
};

bool UHCIController::interrupt(Ints::State *state, void *context)
{
    UHCIController *uhci = (UHCIController *)context;
    uint16_t sts = uhci->readw(USBSTS);
    if(!(sts & 0x1F)) return false;

    int16_t frNum = (uhci->readw(FRNUM) - 1) & 0x3FF;
    printf("[uhci] interrupt (%#.4x %#.4x %#.4x)\n", uhci->base, sts, frNum);

    //uhci->frameList[frNum] |= 1;

    if(sts & (STS_HCHALTED | STS_PROC_ERR | STS_SYS_ERR | STS_USB_ERROR))
    {
        if(sts & STS_HCHALTED) printf("[uhci] error: halted\n");
        if(sts & STS_PROC_ERR) printf("[uhci] error: process error\n");
        if(sts & STS_SYS_ERR) printf("[uhci] error: system error\n");
        if(sts & STS_USB_ERROR) printf("[uhci] error: usb error\n");
    }
    else if(sts & (STS_USBINT))
    {        
    }

    uhci->writew(USBSTS, 0xFFFF); // clear status flags
    return true;
}

UHCIController::UHCIController(uint16_t base, uint8_t irq) :
    base(base),
    irq(irq),
    interruptHandler { nullptr, interrupt, this },
    frameList(new(PAGE_SIZE) dword[1024]),
    frameListPhAddr(Paging::GetPhysicalAddress(Paging::GetAddressSpace(), (uintptr_t)frameList))
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

    Time::Sleep(1000, false);

    reset();
    writed(FRBASEADD, frameListPhAddr); // set frame list address
    writew(USBSTS, 0xFFFF); // clear all possible status bits
    enableInterrupts();

    // initialize frame list
    for(int i = 0; i < 1024; ++i)
        frameList[i] = 1;

    start();

    // just for testing
    setw(PORTSC, 4);
    Time::Sleep(100, false);

    unsigned char buf[64];
    memset(buf, 0, sizeof(buf));
    USBSetupPacket sp;
    sp.bmRequestType = USB_RT_DIR_D2H | USB_RT_TYPE_STANDARD | USB_RT_RECIPIENT_DEVICE;
    sp.bRequest = USB_REQUEST_GET_DESCRIPTOR;
    sp.wValue = USB_DESCRIPTOR_STRING << 8 | 2;
    sp.wIndex = 0;
    sp.wLength = sizeof(buf);

    int res = ControlTransfer(nullptr, &sp, buf, true, sp.wLength, 0);
    Time::Sleep(100, false);
    printf("res: %d\n", res);

    for(int i = 0; i < sizeof(buf); ++i)
        printf("%.2x%s", buf[i], (i & 7) == 7 ? "\n" : " ");
    printf("%-62S\n", buf + 2);
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
}

void UHCIController::start()
{
    setw(USBCMD, CMD_RS);
}

void UHCIController::schedule(QueueHead *qh)
{
    uintptr_t addrSpace = Paging::GetAddressSpace();
    uintptr_t qhPhAddr = Paging::GetPhysicalAddress(addrSpace, (uintptr_t)qh);

    // just for testing !!!
    qh->HLP |= 3;
    stop();
    int frNum = readw(FRNUM) & 1023;
    //printf("qh->hlp:%.8x qh->elp:%.8x frnum: %d\n", qh->HLP, qh->ELP, frNum);
    frameList[frNum] = qhPhAddr | 2;
    start();
}

QueueHead *UHCIController::allocQH()
{   // simple new for testing only
    return new (32) QueueHead;
}

void UHCIController::freeQH(QueueHead *qh)
{
    delete qh;
}

void UHCIController::freeQHandTDs(QueueHead *qh)
{
    if(!qh) return;
    for(TransferDescriptor *td = qh->Child; td;)
    {
        TransferDescriptor *nextTD = td->Next;
        freeTD(td);
        td = nextTD;
    }
    freeQH(qh);
}

TransferDescriptor *UHCIController::allocTD()
{
    // 32 byte alignment until proper dma buffer allocation is implemented
    return new (32) TransferDescriptor;
}

TransferDescriptor *UHCIController::allocTD(uintptr_t link, bool vf, bool t, bool ioc, bool iso, bool ls, uint8_t pid, uint8_t addr, uint8_t endpt, bool d, size_t maxLen, uintptr_t buffer)
{
    // 32 byte alignment until proper dma buffer allocation is implemented
    return new (32) TransferDescriptor(link, vf, t, ioc, iso, ls, pid, addr, endpt, d, maxLen, buffer);
}

void UHCIController::freeTD(TransferDescriptor *td)
{
    delete td;
}

UHCIController::~UHCIController()
{
    // global reset
    _outw(base + USBCMD, _inw(base + USBCMD) | 0x0002);
    Time::Sleep(10, false);
    _outw(base + USBCMD, _inw(base + USBCMD) & ~0x0002);

    IRQs::TryDisable(irq);
    IRQs::UnRegisterHandler(irq, &interruptHandler);

    delete[] frameList;
}

void UHCIController::Initialize()
{
    if(!PCI::Lock->Acquire(0, false))
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
    PCI::Lock->Release();
}

void UHCIController::Cleanup()
{

}

void UHCIController::Probe()
{

}

int UHCIController::ControlTransfer(USBDevice *device, USBSetupPacket *setupPacket, void *buffer, bool in, size_t n, uint8_t endpoint)
{
    UHCIDevice *uhciDevice = (UHCIDevice *)device;

    QueueHead *qh = allocQH();
    if(!qh) return -ENOMEM;

    bool ls = uhciDevice ? uhciDevice->LowSpeed : false;
    bool d = false;

    uint8_t address = device ? device->Address : 0;

    uintptr_t addrSpace = Paging::GetAddressSpace();
    uintptr_t setupPacketPhAddr = Paging::GetPhysicalAddress(addrSpace, (uintptr_t)setupPacket);
    TransferDescriptor *setup = allocTD(0, true, false, false, false, ls, USB_PID_SETUP, address, endpoint, d, sizeof(USBSetupPacket), setupPacketPhAddr);
    d = !d;
    if(!setup)
    {
        freeQHandTDs(qh);
        return -ENOMEM;
    }
    qh->Child = setup;

    TransferDescriptor *currTD = setup;
    if(n)
    {   // allocate data TDs
        uintptr_t bufferPhAddr = Paging::GetPhysicalAddress(addrSpace, (uintptr_t)buffer);
        int maxBytesPerPacket = ls ? 8 : 64;
        int tdCount = align(n, maxBytesPerPacket) / maxBytesPerPacket;

        uint8_t pid = in ? USB_PID_IN : USB_PID_OUT;
        for(int i = 0; i < tdCount; ++i)
        {
            TransferDescriptor *td = allocTD(0, true, false, false, false, ls, pid, address, endpoint, d, n, bufferPhAddr + i * maxBytesPerPacket);
            if(!td)
            {
                freeQHandTDs(qh);
                return -ENOMEM;
            }
            d = !d;
            currTD->Next = td;
            currTD = currTD->Next;
        }
    }

    // allocate handshake TD
    TransferDescriptor *handshake = allocTD(0, false, true, true, false, ls, in ? USB_PID_OUT : USB_PID_IN, address, endpoint, true, 0, 0);
    if(!handshake)
    {
        freeQHandTDs(qh);
        return -ENOMEM;
    }
    currTD->Next = handshake;
    currTD = currTD->Next;

    // calculate physical addresses
    for(TransferDescriptor *td = qh->Child; td->Next; td = td->Next)
        td->LP |= Paging::GetPhysicalAddress(addrSpace, (uintptr_t)td->Next);
    qh->ELP = Paging::GetPhysicalAddress(addrSpace, (uintptr_t)qh->Child);

    // schedule that transfer
    schedule(qh);

    return 0;
}
