#include <cmi8738.h>
#include <pci.h>
#include <mutex.h>
#include <stdio.h>

bool CMI8738::interrupt(Ints::State *state, void *context)
{
    return false;
}

void CMI8738::Initialize()
{
    PCI::Lock->Acquire(0, false);
    for(PCI::Device *pciDev : *PCI::Devices)
    {
        if(pciDev->VendorID != 0x13F6 || pciDev->DeviceID != 0x0111)
            continue;

        printf("[cmi8738] Found compatible device at PCI:%d.%d.%d\n",
               PCI_ADDR_BUS(pciDev->Address),
               PCI_ADDR_DEV(pciDev->Address),
               PCI_ADDR_FUNC(pciDev->Address));


    }
    PCI::Lock->Release();
}

void CMI8738::Cleanup()
{

}
