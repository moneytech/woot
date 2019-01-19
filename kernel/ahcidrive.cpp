#include <ahcidrive.h>
#include <cpu.h>
#include <ints.h>
#include <irqs.h>
#include <list.h>
#include <mutex.h>
#include <pci.h>
#include <stdio.h>

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
    uint32_t clb;       // 0x00, command list base address, 1K-byte aligned
    uint32_t clbu;      // 0x04, command list base address upper 32 bits
    uint32_t fb;        // 0x08, FIS base address, 256-byte aligned
    uint32_t fbu;       // 0x0C, FIS base address upper 32 bits
    uint32_t is;        // 0x10, interrupt status
    uint32_t ie;        // 0x14, interrupt enable
    uint32_t cmd;       // 0x18, command and status
    uint32_t rsv0;      // 0x1C, Reserved
    uint32_t tfd;       // 0x20, task file data
    uint32_t sig;       // 0x24, signature
    uint32_t ssts;      // 0x28, SATA status (SCR0:SStatus)
    uint32_t sctl;      // 0x2C, SATA control (SCR2:SControl)
    uint32_t serr;      // 0x30, SATA error (SCR1:SError)
    uint32_t sact;      // 0x34, SATA active (SCR3:SActive)
    uint32_t ci;        // 0x38, command issue
    uint32_t sntf;      // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t fbs;       // 0x40, FIS-based switch control
    uint32_t rsv1[11];  // 0x44 ~ 0x6F, Reserved
    uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific
};

struct HBA_MEM
{
    uint32_t cap;                   // 0x00, Host capability
    uint32_t ghc;                   // 0x04, Global host control
    uint32_t is;                    // 0x08, Interrupt status
    uint32_t pi;                    // 0x0C, Port implemented
    uint32_t vs;                    // 0x10, Version
    uint32_t ccc_ctl;               // 0x14, Command completion coalescing control
    uint32_t ccc_pts;               // 0x18, Command completion coalescing ports
    uint32_t em_loc;                // 0x1C, Enclosure management location
    uint32_t em_ctl;                // 0x20, Enclosure management control
    uint32_t cap2;                  // 0x24, Host capabilities extended
    uint32_t bohc;                  // 0x28, BIOS/OS handoff control and status
    uint8_t rsv[0xA0 - 0x2C];       // Reserved
    uint8_t vendor[0x100 - 0xA0];   // Vendor specific registers
    HBA_PORT ports[32];             // Port control registers
};

struct HBA_CMD_HEADER
{
    uint8_t cfl : 5;    // Command FIS length in DWORDS, 2 ~ 16
    uint8_t a : 1;      // ATAPI
    uint8_t w : 1;      // Write, 1: H2D, 0: D2H
    uint8_t p : 1;      // Prefetchable
    uint8_t r : 1;      // Reset
    uint8_t b : 1;      // BIST
    uint8_t c : 1;      // Clear busy upon R_OK
    uint8_t rsv0 : 1;   // Reserved
    uint8_t pmp : 4;    // Port multiplier port
    uint16_t prdtl;     // Physical region descriptor table length in entries
    uint32_t prdbc;     // Physical region descriptor byte count transferred
    uint32_t ctba;      // Command table descriptor base address
    uint32_t ctbau;     // Command table descriptor base address upper 32 bits
    uint32_t rsv1[4];   // Reserved
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

        // enable PCI BusMaster, MMIO address space accesses
        PCI::WriteConfigWord(pciDev->Address | 0x04, PCI::ReadConfigWord(pciDev->Address | 0x04) | 0x0006);

        PCI::Config cfg;
        PCI::ReadConfigData(&cfg, pciDev->Address);

        AHCIDrive::Controller *controller = new AHCIDrive::Controller(cfg.Header.Default.BAR[5] & ~7, cfg.Header.Default.InterruptLine);
        controllers.Append(controller);
    }

    PCI::Lock->Release();

    for(AHCIDrive::Controller *ctrl : controllers)
    {

    }
}

void AHCIDrive::Cleanup()
{
    for(AHCIDrive::Controller *ctrl : controllers)
        delete ctrl;
    controllers.Clear();
}

AHCIDrive::Controller::Controller(uintptr_t base, uint8_t irq) :
    hba((HBA_MEM *)base), irq(irq)
{
}

AHCIDrive::Controller::~Controller()
{
}
