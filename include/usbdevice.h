#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <list.h>
#include <types.h>

class Mutex;
class USBController;

class USBDevice
{
    static List<USBDevice *> devices;
    static Mutex lock;
public:
    USBController *Controller;
    USBDevice *Parent;
    int Port;
    int Address;

    static bool Lock();
    static void UnLock();

    USBDevice(USBController *controller, USBDevice *parent, int port, int address);
    int ControlTransfer(USBSetupPacket *setupPacket, void *buffer, bool in, size_t n, uint8_t endpoint);
    ~USBDevice();
};

#endif // USBDEVICE_H
