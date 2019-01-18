#include <errno.h>
#include <usbcontroller.h>

Sequencer<int> USBController::ids(0);
List<USBController *> USBController::controllers;
Mutex USBController::lock("usb");

void USBController::Initialize()
{

}

bool USBController::Lock()
{
    return lock.Acquire(0, false);
}

int USBController::Add(USBController *controller)
{
    if(!Lock()) return -EBUSY;
    int id = ids.GetNext();
    controller->id = id;
    controllers.Append(controller);
    UnLock();
    return id;
}

USBController *USBController::GetByID_nolock(int id)
{
    for(USBController *usb : controllers)
    {
        if(usb->id == id)
            return usb;
    }
    return nullptr;
}

USBController *USBController::GetByID(int id)
{
    if(!Lock()) return nullptr;
    USBController *res = GetByID_nolock(id);
    UnLock();
    return res;
}

int USBController::RemoveByID_nolock(int id)
{
    USBController *usb = GetByID_nolock(id);
    return usb && controllers.Remove(usb, nullptr, false) ? 0 : -ENOENT;
}

int USBController::RemoveByID(int id)
{
    if(!Lock()) return -EBUSY;
    int res = RemoveByID_nolock(id);
    UnLock();
    return res;
}

void USBController::UnLock()
{
    lock.Release();
}

void USBController::Cleanup()
{
}

int USBController::BitStuffTime(int n)
{
    return n * 7 / 6;
}

void USBController::Probe()
{
}

int USBController::ControlTransfer(USBDevice *device, USBSetupPacket *setupPacket, void *buffer, bool in, size_t n, uint8_t endpoint)
{
    return -ENOSYS;
}
