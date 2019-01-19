#ifndef AHCI_H
#define AHCI_H

#include <drive.h>
#include <ints.h>
#include <list.h>
#include <types.h>

struct HBA_MEM;
struct HBA_PORT;
struct HBA_CMD_HEADER;
struct HBA_FIS;
struct HBA_CMD_TBL;

class AHCIDrive : public Drive
{
    class Port;

    enum class DeviceType
    {
        None = 0,
        Unknown,
        SATA,
        SATAPI,
        PM,
        SEMB
    };

    class Controller
    {
    public:
        volatile HBA_MEM *Registers;
        uint8_t IRQ;
        Port *Ports[32];

        Controller(uintptr_t base, uint8_t irq);
        void Enable();
        void Disable();
        void EnableInterrupts();
        void DisableInterrupts();
        void Reset();
        ~Controller();
    };

    class Port
    {
        Controller *Parent;
        int PortNumber;
        volatile HBA_PORT *Registers;
        volatile HBA_CMD_HEADER *CmdHeader;
        volatile HBA_FIS *FIS;
        volatile HBA_CMD_TBL *CmdTable;
        size_t MaxPRDTs = 256;
        size_t CmdTableSize;
    public:
        Port(Controller *controller, int portNumber);
        DeviceType GetDeviceType();
        int StartCommandEngine();
        int StopCommandEngine();
        int Rebase();
        ~Port();
    };

    static List<AHCIDrive::Controller *> controllers;

public:
    static void Initialize();
    static void Cleanup();
};

#endif // AHCI_H
