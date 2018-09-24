#include <drive.h>
#include <errno.h>
#include <mutex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volume.h>
#include <volumetype.h>

List<VolumeType *> *VolumeType::volumeTypes = nullptr;
Mutex *VolumeType::listLock = nullptr;

void VolumeType::Initialize()
{
    volumeTypes = new List<VolumeType *>();
    listLock = new Mutex();
}

bool VolumeType::LockList()
{
    return listLock->Acquire(0, false);
}

bool VolumeType::Add(VolumeType *type)
{
    if(!LockList()) return false;
    VolumeType *t = GetByName(type->Name);
    if(t)
    {
        printf("[volumetype] Type '%s' already exists\n", type->Name);
        UnLockList();
        return false;
    }
    volumeTypes->Prepend(type);
    UnLockList();
    return true;
}

VolumeType *VolumeType::GetByName(const char *name)
{
    if(!LockList()) return nullptr;
    for(VolumeType *type : *volumeTypes)
    {
        if(!strcmp(name, type->Name))
        {
            UnLockList();
            return type;
        }
    }
    UnLockList();
    return nullptr;
}

VolumeType *VolumeType::GetByIndex(uint idx)
{
    if(!LockList()) return nullptr;
    auto res = volumeTypes->Get(idx);
    UnLockList();
    return res;
}

void VolumeType::Remove(VolumeType *type)
{
    if(!LockList()) return;
    volumeTypes->Remove(type, nullptr, false);
    UnLockList();
}

int VolumeType::AutoDetect()
{
    if(!Drive::LockList()) return -1;
    int res = 0;
    for(uint i = 0;; ++i)
    {
        Drive *drive = Drive::GetByIndex(i);
        if(!drive) break;
        if(!LockList()) continue;
        for(VolumeType *v : *volumeTypes)
        {
            int detected = v->Detect(drive);
            if(detected < 0) continue;
            res += detected;
        }
        UnLockList();
    }
    Drive::UnLockList();
    return res;
}

void VolumeType::UnLockList()
{
    listLock->Release();
}

void VolumeType::Cleanup()
{
    listLock->Acquire(0, false);
    for(auto v : *volumeTypes)
        delete v;
    delete volumeTypes;
    delete listLock;
}

VolumeType::VolumeType(const char *name) :
    Name(strdup(name))
{
}

int VolumeType::Detect(Drive *drive)
{
    return -ENOSYS;
}

bool VolumeType::Compare(Volume *a, Volume *b)
{
    return false;
}

VolumeType::~VolumeType()
{
    if(Name) free(Name);
}
