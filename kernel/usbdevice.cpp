#include <mutex.h>
#include <usbcontroller.h>
#include <usbdevice.h>

List<USBDevice *> USBDevice::devices;
Mutex USBDevice::lock("usbdevice");

bool USBDevice::Lock()
{
    return lock.Acquire(0, false);
}

void USBDevice::UnLock()
{
    lock.Release();
}

USBDevice::USBDevice(USBController *controller, USBDevice *parent, int port, int address) :
    Controller(controller),
    Parent(parent),
    Port(port),
    Address(address)
{
}

int USBDevice::ControlTransfer(USBSetupPacket *setupPacket, void *buffer, bool in, size_t n, uint8_t endpoint)
{
    return Controller->ControlTransfer(this, setupPacket, buffer, in, n, endpoint);
}

USBDevice::~USBDevice()
{
}
