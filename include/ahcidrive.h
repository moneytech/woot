#ifndef AHCI_H
#define AHCI_H

#include <drive.h>
#include <ints.h>
#include <list.h>
#include <types.h>

struct HBA_MEM;

class AHCIDrive : public Drive
{
    class Controller
    {
        volatile HBA_MEM *hba;
        uint8_t irq;
    public:
        Controller(uintptr_t base, uint8_t irq);
        ~Controller();
    };
    static List<AHCIDrive::Controller *> controllers;
public:
    static void Initialize();
    static void Cleanup();
};

#endif // AHCI_H
