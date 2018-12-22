#ifndef UHCICONTROLLER_H
#define UHCICONTROLLER_H

#include <ints.h>
#include <types.h>
#include <usbcontroller.h>

class UHCIController : public USBController
{
    uint16_t base;
    uint8_t irq;
    int portCount;
    Ints::Handler interruptHandler;
    uint32_t *frameList;
    uintptr_t frameListPhAddr;
    struct ETD *ETDs;

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
    struct ETD *allocETD();
    void freeETD(struct ETD *etd);
    ~UHCIController();
protected:
    virtual int Transfer(void *buffer, int n, uint8_t pid, uint8_t address, uint8_t endpoint);
public:
    static void Initialize();
    static void Cleanup();
};

#endif // UHCICONTROLLER_H
