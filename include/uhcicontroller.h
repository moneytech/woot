#ifndef UHCICONTROLLER_H
#define UHCICONTROLLER_H

#include <ints.h>
#include <types.h>
#include <usbcontroller.h>

struct QueueHead;
struct TransferDescriptor;

class UHCIController : public USBController
{
    uint16_t base;
    uint8_t irq;
    int portCount;
    Ints::Handler interruptHandler;
    uint32_t *frameList;
    uintptr_t frameListPhAddr;

    static bool interrupt(Ints::State *state, void *context);
    UHCIController(uint16_t base, uint8_t irq);
    uint16_t readw(uint16_t reg);
    void writew(uint16_t reg, uint16_t val);
    void writed(uint16_t reg, uint32_t val);
    void setw(uint16_t reg, uint16_t mask);
    void clrw(uint16_t reg, uint16_t mask);
    void reset();
    void enableInterrupts();
    void stop();
    void start();
    void schedule(QueueHead *qh);
    QueueHead *allocQH();
    void freeQH(QueueHead *qh);
    void freeQHandTDs(QueueHead *qh);
    TransferDescriptor *allocTD();
    TransferDescriptor *allocTD(uintptr_t link, bool vf, bool t, bool ioc, bool iso, bool ls, uint8_t pid, uint8_t addr, uint8_t endpt, bool d, size_t maxLen, uintptr_t buffer);
    void freeTD(TransferDescriptor *td);
    ~UHCIController();
public:
    static void Initialize();
    static void Cleanup();

    virtual void Probe();
    virtual int ControlTransfer(USBDevice *device, USBSetupPacket *setupPacket, void *buffer, bool in, size_t n, uint8_t endpoint);
};

#endif // UHCICONTROLLER_H
