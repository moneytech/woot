#include <cpu.h>
#include <drive.h>
#include <errno.h>
#include <mutex.h>
#include <stdlib.h>
#include <string.h>

Sequencer<int> Drive::id(0);
List<Drive *> *Drive::drives;
Mutex *Drive::lock;

void Drive::Initialize()
{
    drives = new List<Drive *>();
    lock = new Mutex();
}

bool Drive::LockList()
{
    return lock->Acquire(0, false);
}

Drive *Drive::GetByID(int id)
{
    if(!LockList()) return nullptr;
    Drive *res = nullptr;
    for(auto it : *drives)
    {
        if(it->ID == id)
        {
            res = it;
            break;
        }
    }
    UnLockList();
    return res;
}

Drive *Drive::GetByIndex(uint idx)
{
    if(!LockList()) return nullptr;
    Drive *drive = drives->Get(idx);
    UnLockList();
    return drive;
}

int Drive::Add(Drive *drive)
{
    if(!LockList()) return -1;
    int newId = id.GetNext();
    drives->Append(drive);
    drive->ID = newId;
    UnLockList();
    return newId;
}

bool Drive::Remove(Drive *drive)
{
    if(!LockList()) return false;
    bool res = drives->Remove(drive, nullptr, false) != 0;
    drive->ID = -1;
    UnLockList();
    return res;
}

void Drive::UnLockList()
{
    lock->Release();
}

void Drive::Cleanup()
{
    LockList();
    for(auto n : *drives)
        delete n;
    delete drives;
    delete lock;
}

Drive::Drive(size_t sectorSize, size64_t sectorCount, const char *model, const char *serial) :
    ID(-1),
    SectorSize(sectorSize),
    SectorCount(sectorCount),
    Size(SectorSize * SectorCount),
    Model(model ? strdup(model) : nullptr),
    Serial(serial ? strdup(serial) : nullptr)
{
}

int64_t Drive::ReadSectors(void *buffer, uint64_t start, int64_t count)
{
    return -ENOSYS;
}

int64_t Drive::WriteSectors(const void *buffer, uint64_t start, int64_t count)
{
    return -ENOSYS;
}

bool Drive::HasMedia()
{
    return false;
}

Drive::~Drive()
{
    if(Model) free(Model);
    if(Serial) free(Serial);
}
