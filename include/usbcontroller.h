#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H

#include <list.h>
#include <mutex.h>
#include <sequencer.h>
#include <types.h>

#define USB_PID_OUT     0xE1
#define USB_PID_IN      0x69
#define USB_PID_SOF     0xA5
#define USB_PID_SETUP   0x2D
#define USB_PID_DATA0   0xC3
#define USB_PID_DATA1   0x4B
#define USB_PID_DATA2   0x87
#define USB_PID_MDATA   0x0F
#define USB_PID_ACK     0xD2
#define USB_PID_NAK     0x5A
#define USB_PID_STALL   0x1E
#define USB_PID_NYET    0x96
#define USB_PID_PRE     0x3C
#define USB_PID_SPLIT   0x78
#define USB_PID_PING    0xB4

#define USB_RT_DIR_H2D              0
#define USB_RT_DIR_D2H              (1 << 7)
#define USB_RT_TYPE_STANDARD        0
#define USB_RT_TYPE_CLASS           (1 << 5)
#define USB_RT_TYPE_VENDOR          (2 << 5)
#define USB_RT_RECIPIENT_DEVICE     0
#define USB_RT_RECIPIENT_INTERFACE  1
#define USB_RT_RECIPIENT_ENDPOINT   2
#define USB_RT_RECIPIENT_OTHER      3

#define USB_REQUEST_GET_STATUS          0
#define USB_REQUEST_CLEAR_FEATURE       1
#define USB_REQUEST_SET_FEATURE         3
#define USB_REQUEST_SET_ADDRESS         5
#define USB_REQUEST_GET_DESCRIPTOR      6
#define USB_REQUEST_SET_DESCRIPTOR      7
#define USB_REQUEST_GET_CONFIGURATION   8
#define USB_REQUEST_SET_CONFIGURATION   9
#define USB_REQUEST_GET_INTERFACE       10
#define USB_REQUEST_SET_INTERFACE       11
#define USB_REQUEST_SYNCH_FRAME         12

#define USB_DESCRIPTOR_DEVICE           1
#define USB_DESCRIPTOR_CONFIGURATION    2
#define USB_DESCRIPTOR_STRING           3
#define USB_DESCRIPTOR_INTERFACE        4
#define USB_DESCRIPTOR_ENDPOINT         5

#pragma pack(push, 1)
struct USBSetupPacket
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
};

struct USBUnicodeString
{
    uint8_t bLength;
    uint8_t bDescriptorType;
};
#pragma pack(pop)

class USBDevice;

class USBController
{
    static Sequencer<int> ids;
    static List<USBController *> controllers;
    static Mutex lock;

    int id = -1;
public:
    static void Initialize();
    static bool Lock();
    static int Add(USBController *controller);
    static USBController *GetByID_nolock(int id);
    static USBController *GetByID(int id);
    static int RemoveByID_nolock(int id);
    static int RemoveByID(int id);
    static void UnLock();
    static void Cleanup();

    virtual void Probe();
    virtual int ControlTransfer(USBDevice *device, USBSetupPacket *setupPacket, void *buffer, bool in, size_t n, uint8_t endpoint);
};

#endif // USBCONTROLLER_H
