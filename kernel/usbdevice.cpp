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

int USBDevice::ControlTransfer(USBSetupPacket *setupPacket, void *buffer, bool in, size_t n, uint8_t endpoint)
{
    return Controller->ControlTransfer(setupPacket, buffer, in, n, Address, endpoint);
}
