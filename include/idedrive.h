#ifndef IDEDRIVE_H
#define IDEDRIVE_H

#include <ata.h>
#include <drive.h>
#include <ints.h>
#include <types.h>

class Mutex;
class Semaphore;

class IDEDrive : public Drive
{
    class Controller
    {
        friend class IDEDrive;

        Mutex *Lock;
        word Base, Control, BM;
        byte IRQ;
        Ints::Handler InterruptHandler;
        IDEDrive *Drives[2];

        bool waitFornBSYorERR_nolock(int timeout, byte *status);
        bool waitForDRQorERR_nolock(int timeout, byte *status);
        bool waitFornBSY_nolock(int timeout, byte *status);
        bool waitForDRDY_nolock(int timeout, byte *status);
        bool identify_nolock(ATAIdentifyResponse *id, bool slave, bool atapi);

        Controller(word base, word control, word bm, byte irq);
        void EnableIRQ();
        void DisableIRQ();
        void Wait400ns();
        void SoftReset();
        bool WaitFornBSYorERR(int timeout, byte *status);
        bool WaitForDRQorERR(int timeout, byte *status);
        bool WaitFornBSY(int timeout, byte *status);
        bool WaitForDRDY(int timeout, byte *status);
        bool Identify(ATAIdentifyResponse *id, bool slave, bool atapi);
    };

#pragma pack(push, 1)
    typedef struct PRDTEntry
    {
        dword BufferAddress;
        word ByteCount;
        word Reserved : 15;
        word Last : 1;
    } PRDTEntry;
#pragma pack(pop)

    static const word defAddrs[4];

    Controller *Controller;
    bool Slave, ATAPI;
    ATAIdentifyResponse IDResp;
    size_t PRDTsAllocated;
    PRDTEntry *PRDTs;
    Semaphore *TransferDone;
    int MaxBlockTransfer;

    static bool interrupt(Ints::State *state, void *context);

    IDEDrive(ATAIdentifyResponse *id, bool atapi, class Controller *ctrl, bool slave);
    char *getStringFromID(ATAIdentifyResponse *id, uint offset, uint length);
    int sectorTransfer(bool write, void *buffer, uint64_t start, int64_t count);
public:
    static void Initialize();
    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count);
    virtual ~IDEDrive();
};

#endif // IDEDRIVE_H
