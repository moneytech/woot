#ifndef UHCIDEVICE_H
#define UHCIDEVICE_H

#include <types.h>
#include <usbdevice.h>

class UHCIDevice : public USBDevice
{
public:
    bool LowSpeed;

    UHCIDevice(USBController *controller, USBDevice *parent, int port, int address, bool lowSpeed);
    ~UHCIDevice();
};

#endif // UHCIDEVICE_H
