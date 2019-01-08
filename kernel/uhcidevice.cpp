#include <uhcidevice.h>

UHCIDevice::UHCIDevice(USBController *controller, USBDevice *parent, int port, int address, bool lowSpeed) :
    USBDevice(controller, parent, port, address),
    LowSpeed(lowSpeed)
{
}

UHCIDevice::~UHCIDevice()
{
}
