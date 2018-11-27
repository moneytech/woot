#include <drive.h>
#include <errno.h>
#include <mutex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volume.h>
#include <volumetype.h>

List<VolumeType *> VolumeType::volumeTypes;
Mutex VolumeType::listLock("volumeTypeList");

VolumeType *VolumeType::getByName_nolock(const char *name)
{
    for(VolumeType *type : volumeTypes)
    {
        if(!strcmp(name, type->Name))
            return type;
    }
    return nullptr;
}

void VolumeType::Initialize()
{
}

bool VolumeType::LockList()
{
    return listLock.Acquire(0, false);
}

bool VolumeType::Add(VolumeType *type)
{
    if(!LockList()) return false;
    VolumeType *t = getByName_nolock(type->Name);
    if(t)
    {
        printf("[volumetype] Type '%s' already exists\n", type->Name);
        UnLockList();
        return false;
    }
    volumeTypes.Prepend(type);
    UnLockList();
    return true;
}

VolumeType *VolumeType::GetByName(const char *name)
{
    if(!LockList()) return nullptr;
    VolumeType *vt = getByName_nolock(name);
    UnLockList();
    return vt;
}

VolumeType *VolumeType::GetByIndex(uint idx)
{
    if(!LockList()) return nullptr;
    VolumeType *res = volumeTypes[idx];
    UnLockList();
    return res;
}

void VolumeType::Remove(VolumeType *type)
{
    if(!LockList()) return;
    volumeTypes.Remove(type, nullptr, false);
    UnLockList();
}

int VolumeType::AutoDetect()
{
    int res = 0;
    for(uint i = 0;; ++i)
    {
        Drive *drive = Drive::GetByIndex(i);
        if(!drive) break;
        if(!LockList()) continue;
        for(VolumeType *v : volumeTypes)
        {
            int detected = v->Detect(drive);
            if(detected < 0) continue;
            res += detected;
        }
        UnLockList();
    }
    return res;
}

void VolumeType::UnLockList()
{
    listLock.Release();
}

void VolumeType::Cleanup()
{
    LockList();
    for(auto v : volumeTypes)
        delete v;
    UnLockList();
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
