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

int USBDevice::Transfer(void *buffer, int n, uint8_t pid, uint8_t endpoint)
{
    return Controller->Transfer(buffer, n, pid, Address, endpoint);
}
