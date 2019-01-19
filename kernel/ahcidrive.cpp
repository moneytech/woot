#include <ahcidrive.h>
#include <cpu.h>
#include <errno.h>
#include <ints.h>
#include <irqs.h>
#include <list.h>
#include <mutex.h>
#include <paging.h>
#include <pci.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#pragma pack(push, 1)
struct FIS_REG_H2D
{
    uint8_t fis_type;   // FIS_TYPE_REG_H2D
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0:3;     // Reserved
    uint8_t c:1;        // 1: Command, 0: Control
    uint8_t command;    // Command register
    uint8_t featurel;   // Feature register, 7:0
    uint8_t lba0;       // LBA low register, 7:0
    uint8_t lba1;       // LBA mid register, 15:8
    uint8_t lba2;       // LBA high register, 23:16
    uint8_t device;     // Device register
    uint8_t lba3;       // LBA register, 31:24
    uint8_t lba4;       // LBA register, 39:32
    uint8_t lba5;       // LBA register, 47:40
    uint8_t featureh;   // Feature register, 15:8
    uint8_t countl;     // Count register, 7:0
    uint8_t counth;     // Count register, 15:8
    uint8_t icc;        // Isochronous command completion
    uint8_t control;    // Control register
    uint8_t rsv1[4];    // Reserved
};

struct FIS_REG_D2H
{
    uint8_t fis_type;   // FIS_TYPE_REG_D2H
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 2;   // Reserved
    uint8_t i : 1;      // Interrupt bit
    uint8_t rsv1 : 1;   // Reserved
    uint8_t status;     // Status register
    uint8_t error;      // Error register
    uint8_t lba0;       // LBA low register, 7:0
    uint8_t lba1;       // LBA mid register, 15:8
    uint8_t lba2;       // LBA high register, 23:16
    uint8_t device;     // Device register
    uint8_t lba3;       // LBA register, 31:24
    uint8_t lba4;       // LBA register, 39:32
    uint8_t lba5;       // LBA register, 47:40
    uint8_t rsv2;       // Reserved
    uint8_t countl;     // Count register, 7:0
    uint8_t counth;     // Count register, 15:8
    uint8_t rsv3[2];    // Reserved
    uint8_t rsv4[4];    // Reserved
};

struct FIS_DATA
{
    uint8_t fis_type;   // FIS_TYPE_DATA
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 4;   // Reserved
    uint8_t rsv1[2];    // Reserved
};

struct FIS_PIO_SETUP
{
    uint8_t fis_type;   // FIS_TYPE_PIO_SETUP
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 1;   // Reserved
    uint8_t d : 1;      // Data transfer direction, 1 - device to host
    uint8_t i : 1;      // Interrupt bit
    uint8_t rsv1 : 1;
    uint8_t status;     // Status register
    uint8_t error;      // Error register
    uint8_t lba0;       // LBA low register, 7:0
    uint8_t lba1;       // LBA mid register, 15:8
    uint8_t lba2;       // LBA high register, 23:16
    uint8_t device;     // Device register
    uint8_t lba3;       // LBA register, 31:24
    uint8_t lba4;       // LBA register, 39:32
    uint8_t lba5;       // LBA register, 47:40
    uint8_t rsv2;       // Reserved
    uint8_t countl;     // Count register, 7:0
    uint8_t counth;     // Count register, 15:8
    uint8_t rsv3;       // Reserved
    uint8_t e_status;   // New value of status register
    uint16_t tc;        // Transfer count
    uint8_t rsv4[2];    // Reserved
};

struct FIS_DMA_SETUP
{
    uint8_t fis_type;       // FIS_TYPE_DMA_SETUP
    uint8_t pmport : 4;     // Port multiplier
    uint8_t rsv0 : 1;       // Reserved
    uint8_t d : 1;          // Data transfer direction, 1 - device to host
    uint8_t i : 1;          // Interrupt bit
    uint8_t a : 1;          // Auto-activate. Specifies if DMA Activate FIS is needed
    uint8_t rsved[2];       // Reserved
    uint64_t DMAbufferID;   // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
    uint32_t rsvd;          // More reserved
    uint32_t DMAbufOffset;  // Byte offset into buffer. First 2 bits must be 0
    uint32_t TransferCount; // Number of bytes to transfer. Bit 0 must be 0
    uint32_t resvd;         // Reserved
};

struct HBA_PORT
{
    uint32_t CLB;           // 0x00, command list base address, 1K-byte aligned
    uint32_t CLBU;          // 0x04, command list base address upper 32 bits
    uint32_t FB;            // 0x08, FIS base address, 256-byte aligned
    uint32_t FBU;           // 0x0C, FIS base address upper 32 bits
    uint32_t IS;            // 0x10, interrupt status
    uint32_t IE;            // 0x14, interrupt enable
    uint32_t CMD;           // 0x18, command and status
    uint32_t Reserved0;     // 0x1C, Reserved
    uint32_t TFD;           // 0x20, task file data
    uint32_t SIG;           // 0x24, signature
    uint32_t SSTS;          // 0x28, SATA status (SCR0:SStatus)
    uint32_t SCTL;          // 0x2C, SATA control (SCR2:SControl)
    uint32_t SERR;          // 0x30, SATA error (SCR1:SError)
    uint32_t SACT;          // 0x34, SATA active (SCR3:SActive)
    uint32_t CI;            // 0x38, command issue
    uint32_t SNTF;          // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t FBS;           // 0x40, FIS-based switch control
    uint32_t Reserved1[11]; // 0x44 ~ 0x6F, Reserved
    uint32_t Vendor[4];     // 0x70 ~ 0x7F, vendor specific
};

struct HBA_MEM
{
    uint32_t CAP;                   // 0x00, Host capability
    uint32_t GHC;                   // 0x04, Global host control
    uint32_t IS;                    // 0x08, Interrupt status
    uint32_t PI;                    // 0x0C, Port implemented
    uint32_t VS;                    // 0x10, Version
    uint32_t CCC_CTL;               // 0x14, Command completion coalescing control
    uint32_t CCC_PORTS;             // 0x18, Command completion coalescing ports
    uint32_t EM_LOC;                // 0x1C, Enclosure management location
    uint32_t EM_CTL;                // 0x20, Enclosure management control
    uint32_t CAP2;                  // 0x24, Host capabilities extended
    uint32_t BOHC;                  // 0x28, BIOS/OS handoff control and status
    uint8_t Reserved[0xA0 - 0x2C];  // Reserved
    uint8_t Vendor[0x100 - 0xA0];   // Vendor specific registers
    HBA_PORT Ports[32];             // Port control registers
};

struct HBA_CMD_HEADER
{
    uint8_t CFL : 5;        // Command FIS length in DWORDS, 2 ~ 16
    uint8_t A : 1;          // ATAPI
    uint8_t W : 1;          // Write, 1: H2D, 0: D2H
    uint8_t P : 1;          // Prefetchable
    uint8_t R : 1;          // Reset
    uint8_t B : 1;          // BIST
    uint8_t C : 1;          // Clear busy upon R_OK
    uint8_t Reserved0 : 1;  // Reserved
    uint8_t PMP : 4;        // Port multiplier port
    uint16_t PRDTL;         // Physical region descriptor table length in entries
    uint32_t PRDBC;         // Physical region descriptor byte count transferred
    uint32_t CTBA;          // Command table descriptor base address
    uint32_t CTBAU;         // Command table descriptor base address upper 32 bits
    uint32_t Reserved1[4];  // Reserved
};

struct HBA_PRDT_ENTRY
{
    uint32_t dba;       // Data base address
    uint32_t dbau;      // Data base address upper 32 bits
    uint32_t rsv0;      // Reserved
    uint32_t dbc : 22;  // Byte count, 4M max
    uint32_t rsv1 : 9;  // Reserved
    uint32_t i : 1;     // Interrupt on completion
};

struct HBA_CMD_TBL
{
    uint8_t cfis[64];   // Command FIS
    uint8_t acmd[16];   // ATAPI command, 12 or 16 bytes
    uint8_t rsv[48];    // Reserved
};

typedef uint64_t FIS_DEV_BITS;

struct HBA_FIS
{
    FIS_DMA_SETUP dsfis;    // DMA Setup FIS
    uint8_t pad0[4];
    FIS_PIO_SETUP psfis;    // PIO Setup FIS
    uint8_t pad1[12];
    FIS_REG_D2H rfis;       // Register – Device to Host FIS
    uint8_t pad2[4];
    FIS_DEV_BITS sdbfis;    // Set Device Bit FIS
    uint8_t ufis[64];
    uint8_t rsv[0x100 - 0xA0];
};
#pragma pack(pop)

#define HBA_GHC_AE      (1 << 31)
#define HBA_GHC_MRSM    (1 << 2)
#define HBA_GHC_IE      (1 << 1)
#define HBA_GHC_HR      (1 << 0)

#define	PORT_SIG_SATA   0x00000101  // SATA drive
#define	PORT_SIG_SATAPI 0xEB140101  // SATAPI drive
#define	PORT_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define	PORT_SIG_PM     0x96690101  // Port multiplier

#define PORT_CMD_ICC_IDLE       (0 << 28)
#define PORT_CMD_ICC_ACTIVE     (1 << 28)
#define PORT_CMD_ICC_PARTIAL    (2 << 28)
#define PORT_CMD_ICC_SLUMBER    (6 << 28)
#define PORT_CMD_ICC_DEV_SLEEP  (8 << 28)
#define PORT_CMD_ASP            (1 << 27)
#define PORT_CMD_ALPE           (1 << 26)
#define PORT_CMD_DLAE           (1 << 25)
#define PORT_CMD_ATAPI          (1 << 24)
#define PORT_CMD_APSTE          (1 << 23)
#define PORT_CMD_FBSCP          (1 << 22)
#define PORT_CMD_ESP            (1 << 21)
#define PORT_CMD_CPD            (1 << 20)
#define PORT_CMD_MPSP           (1 << 19)
#define PORT_CMD_HPCP           (1 << 18)
#define PORT_CMD_PMA            (1 << 17)
#define PORT_CMD_CPS            (1 << 16)
#define PORT_CMD_CR             (1 << 15)
#define PORT_CMD_FR             (1 << 14)
#define PORT_CMD_MPSS           (1 << 13)
#define PORT_CMD_CCS            (31 << 8)   // mask
#define PORT_CMD_FRE            (1 << 4)
#define PORT_CMD_CLO            (1 << 3)
#define PORT_CMD_POD            (1 << 2)
#define PORT_CMD_SUD            (1 << 1)
#define PORT_CMD_ST             (1 << 0)

#define PORT_SSTS_IPM_NOT_PRESENT   0
#define PORT_SSTS_IPM_ACTIVE        1
#define PORT_SSTS_IPM_PARTIAL       2
#define PORT_SSTS_IPM_SLUMBER       6
#define PORT_SSTS_IPM_DEV_SLEEP     8

#define PORT_SSTS_SPD_NOT_PRESENT   0
#define PORT_SSTS_SPD_GEN1          1
#define PORT_SSTS_SPD_GEN2          2
#define PORT_SSTS_SPD_GEN3          3

#define PORT_SSTS_DET_NOT_PRESENT       0
#define PORT_SSTS_DET_PRESENT_NOCOMM    1
#define PORT_SSTS_DET_PRESENT_COMM      3
#define PORT_SSTS_DET_OFFLINE           4

static const char *deviceTypeNames[] =
{
    "null device",
    "unknown device",
    "SATA device",
    "SATAPI device",
    "port multiplier",
    "SEMB device"
};

List<AHCIDrive::Controller *> AHCIDrive::controllers;

void AHCIDrive::Initialize()
{
    if(!PCI::Lock->Acquire(0, false))
        return;

    for(PCI::Device *pciDev : *PCI::Devices)
    {
        if(pciDev->Class != 0x01 || pciDev->SubClass != 0x06)
            continue;

        printf("[ahcidrive] Found AHCI controller at PCI:%d.%d.%d\n",
               PCI_ADDR_BUS(pciDev->Address),
               PCI_ADDR_DEV(pciDev->Address),
               PCI_ADDR_FUNC(pciDev->Address));

        // enable PCI BusMaster and MMIO address space accesses
        PCI::WriteConfigWord(pciDev->Address | 0x04, PCI::ReadConfigWord(pciDev->Address | 0x04) | 0x0006);

        PCI::Config cfg;
        PCI::ReadConfigData(&cfg, pciDev->Address);

        AHCIDrive::Controller *controller = new AHCIDrive::Controller(cfg.Header.Default.BAR[5] & ~7, cfg.Header.Default.InterruptLine);
        controllers.Append(controller);
    }

    PCI::Lock->Release();

    for(AHCIDrive::Controller *ctrl : controllers)
    {
        for(int i = 0; i < 32; ++i)
        {
            Port *port = ctrl->Ports[i];
            if(!port) continue;

            DeviceType devType = port->GetDeviceType();

            if(devType == DeviceType::None)
                continue;

            printf("[ahcidrive] Found %s on port %d\n", deviceTypeNames[(int)devType], i);
            int res = port->Rebase();
            if(res)
            {
                printf("[ahcidrive] Couldn't rebase device on port %d (error: %d)\n", i, res);
                continue;
            }

            // TODO: Create AHCIDrive objects here
        }
    }
}

void AHCIDrive::Cleanup()
{
    for(AHCIDrive::Controller *ctrl : controllers)
        delete ctrl;
    controllers.Clear();
}

AHCIDrive::Controller::Controller(uintptr_t base, uint8_t irq) :
    Registers((HBA_MEM *)base), IRQ(irq)
{
    memset(Ports, 0, sizeof(Ports));

    Enable();

    for(uint32_t i = 0, PI = Registers->PI; PI; ++i, PI >>= 1)
    {
        if(!(PI & 1)) continue;
        Ports[i] = new Port(this, i);
    }
}

void AHCIDrive::Controller::Enable()
{
    Registers->GHC |= HBA_GHC_AE;
}

void AHCIDrive::Controller::Disable()
{
    Registers->GHC = 0;
}

void AHCIDrive::Controller::EnableInterrupts()
{
    Registers->GHC |= HBA_GHC_IE;
}

void AHCIDrive::Controller::DisableInterrupts()
{
    Registers->GHC &= ~HBA_GHC_IE;
}

void AHCIDrive::Controller::Reset()
{
    Registers->GHC = 1;
    while(Registers->GHC & 1)
        Time::Sleep(1, false);
}

AHCIDrive::Controller::~Controller()
{
}

AHCIDrive::Port::Port(AHCIDrive::Controller *controller, int portNumber) :
    Parent(controller), PortNumber(portNumber),
    Registers(controller->Registers->Ports + portNumber)
{
}

AHCIDrive::DeviceType AHCIDrive::Port::GetDeviceType()
{
    uint32_t SSTS = Registers->SSTS;
    uint32_t SIG = Registers->SIG;
    uint8_t IPM = SSTS >> 8 & 0x0F;
    uint8_t DET = SSTS & 0x0F;

    if(DET != PORT_SSTS_DET_PRESENT_COMM || IPM != PORT_SSTS_IPM_ACTIVE)
        return DeviceType::None;

    switch(SIG)
    {
    case PORT_SIG_SATA:
        return DeviceType::SATA;
    case PORT_SIG_SATAPI:
        return DeviceType::SATAPI;
    case PORT_SIG_SEMB:
        return DeviceType::SEMB;
    case PORT_SIG_PM:
        return DeviceType::PM;
    }

    return DeviceType::Unknown;
}

int AHCIDrive::Port::StartCommandEngine()
{
    int retry = 1000;
    while(Registers->CMD & PORT_CMD_CR && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    Registers->CMD |= PORT_CMD_FRE | PORT_CMD_ST;
    return 0;
}

int AHCIDrive::Port::StopCommandEngine()
{
    Registers->CMD &= ~PORT_CMD_ST;
    int retry = 1000;
    while(Registers->CMD & PORT_CMD_CR && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    Registers->CMD &= ~PORT_CMD_FRE;
    return 0;
}

int AHCIDrive::Port::Rebase()
{
    int res = StopCommandEngine();
    if(res) return res;

    // allocate needed structures
    CmdHeader = (HBA_CMD_HEADER *)Paging::AllocDMA(sizeof(HBA_CMD_HEADER));
    FIS = (HBA_FIS *)Paging::AllocDMA(sizeof(HBA_FIS));
    CmdTableSize = sizeof(HBA_CMD_TBL) + MaxPRDTs * sizeof(HBA_PRDT_ENTRY);
    CmdTable = (HBA_CMD_TBL *)Paging::AllocDMA(CmdTableSize);

    // zero allocated structures
    memset((void *)CmdHeader, 0, sizeof(HBA_CMD_HEADER));
    memset((void *)FIS, 0, sizeof(HBA_FIS));
    memset((void *)CmdTable, 0, sizeof(HBA_CMD_TBL));

    // present it to the hardware
    uintptr_t addressSpace = Paging::GetAddressSpace();
    Registers->CLB = Paging::GetPhysicalAddress(addressSpace, (uintptr_t)CmdHeader);
    Registers->CLBU = 0;
    Registers->FB = Paging::GetPhysicalAddress(addressSpace, (uintptr_t)FIS);
    Registers->FBU = 0;
    CmdHeader->CTBA = Paging::GetPhysicalAddress(addressSpace, (uintptr_t)CmdTable);
    CmdHeader->CTBAU = 0;

    return StartCommandEngine();
}

AHCIDrive::Port::~Port()
{
}
