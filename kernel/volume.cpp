#include <errno.h>
#include <filesystem.h>
#include <mutex.h>
#include <string.h>
#include <sysdefs.h>
#include <volume.h>

Sequencer<int> Volume::id(0);
List<Volume *> Volume::volumes;
Mutex Volume::lock;

Volume::Volume(class Drive *drive, VolumeType *type) :
    Drive(drive),
    Type(type)
{
}

Volume::~Volume()
{
}

void Volume::Initialize()
{
}

bool Volume::Lock()
{
    return lock.Acquire(0);
}

Volume *Volume::GetByID(int id)
{
    if(!Lock()) return nullptr;
    Volume *res = nullptr;
    for(Volume *vol : volumes)
    {
        if(vol->ID == id)
        {
            res = vol;
            break;
        }
    }
    UnLock();
    return res;
}

Volume *Volume::GetByIndex(uint idx)
{
    if(!Lock())
        return nullptr;
    Volume *res = volumes[idx];
    UnLock();
    return res;
}

Volume *Volume::GetByLabel(const char *label)
{
    if(!Lock()) return nullptr;
    if(!FileSystem::Lock())
    {
        UnLock();
        return nullptr;
    }
    Volume *res = nullptr;
    char labelBuf[MAX_FS_LABEL_LENGTH + 1];
    for(Volume *v : volumes)
    {
        if(!v->FS) continue;
        bool hasLabel = v->FS->GetLabel(labelBuf, sizeof(labelBuf));
        if(!hasLabel || strcmp(label, labelBuf))
            continue;
        res = v;
        break;
    }
    FileSystem::UnLock();
    UnLock();
    return res;
}

Volume *Volume::GetByUUID(UUID uuid)
{
    if(!Lock()) return nullptr;
    if(!FileSystem::Lock())
    {
        UnLock();
        return nullptr;
    }
    Volume *res = nullptr;
    for(Volume *v : volumes)
    {
        if(!v->FS) continue;
        UUID fsuuid = v->FS->GetUUID();
        if(fsuuid == UUID::nil || uuid != fsuuid)
            continue;
        res = v;
        break;
    }
    FileSystem::UnLock();
    UnLock();
    return res;
}

int Volume::Add(Volume *vol)
{
    if(!Lock())
        return -1;
    int newId = id.GetNext();
    volumes.Append(vol);
    vol->ID = newId;
    UnLock();
    return newId;
}

bool Volume::Remove(Volume *vol)
{
    if(!Lock()) return false;
    bool res = volumes.Remove(vol, nullptr, false) != 0;
    vol->ID = -1;
    UnLock();
    return res;
}

void Volume::UnLock()
{
    lock.Release();
}

void Volume::FlushAll()
{
    Lock();
    for(Volume *v : volumes)
        v->Flush();
    UnLock();
}

void Volume::Cleanup()
{
    Lock();
    for(Volume *v : volumes)
    {
        v->Flush();
        delete v;
    }
    UnLock();
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
