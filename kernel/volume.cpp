#include <errno.h>
#include <mutex.h>
#include <volume.h>

Sequencer<int> Volume::id(0);
List<Volume *> *Volume::volumes;
Mutex *Volume::listLock;

Volume::Volume(class Drive *drive, VolumeType *type) :
    Drive(drive),
    Type(type),
    lock(new Mutex())
{
}

Volume::~Volume()
{
    Lock();
    delete lock;
}

void Volume::Initialize()
{
    volumes = new List<Volume *>();
    listLock = new Mutex();
}

bool Volume::LockList()
{
    return listLock->Acquire(0);
}

Volume *Volume::GetByID(int id, bool lock)
{
    if(!LockList()) return nullptr;
    Volume *res = nullptr;
    for(auto it : *volumes)
    {
        if(it->ID == id)
        {
            res = it;
            break;
        }
    }
    if(res && lock)
    {
        if(!res->Lock())
            res = nullptr;
    }
    UnLockList();
    return res;
}

Volume *Volume::GetByIndex(uint idx, bool lock)
{
    if(!LockList()) return nullptr;
    auto res = volumes->Get(idx);
    if(res && lock)
    {
        if(!res->Lock())
            res = nullptr;
    }
    UnLockList();
    return res;
}

int Volume::Add(Volume *vol)
{
    if(!LockList()) return -1;
    int newId = id.GetNext();
    volumes->Append(vol);
    vol->ID = newId;
    UnLockList();
    return newId;
}

bool Volume::Remove(Volume *vol)
{
    if(!LockList()) return false;
    bool res = volumes->Remove(vol, nullptr, false) != 0;
    vol->ID = -1;
    UnLockList();
    return res;
}

void Volume::UnLockList()
{
    listLock->Release();
}

void Volume::Cleanup()
{
    LockList();
    if(volumes) return;
    for(Volume *v : *volumes)
        delete v;
    delete volumes;
    delete listLock;
}

bool Volume::Lock()
{
    return lock->Acquire(0, false);
}

int64_t Volume::Read(void *buffer, uint64_t position, int64_t n)
{
    return -ENOSYS;
}

int64_t Volume::Write(const void *buffer, uint64_t position, int64_t n)
{
    return -ENOSYS;
}

int64_t Volume::ReadSectors(void *buffer, uint64_t start, int64_t count)
{
    return -ENOSYS;
}

int64_t Volume::WriteSectors(const void *buffer, uint64_t start, int64_t count)
{
    return -ENOSYS;
}

bool Volume::Flush()
{
    return false;
}

void Volume::UnLock()
{
    lock->Release();
}
