#include <errno.h>
#include <filesystem.h>
#include <filesystemtype.h>
#include <mutex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volume.h>

List<FileSystemType *> *FileSystemType::types;
Mutex *FileSystemType::listLock;

void FileSystemType::Initialize()
{
    types = new List<FileSystemType *>();
    listLock = new Mutex();
}

bool FileSystemType::LockList()
{
    return listLock->Acquire(0, false);
}

bool FileSystemType::Add(FileSystemType *type)
{
    if(!LockList()) return false;
    FileSystemType *t = GetByName(type->Name);
    if(t)
    {
        printf("[filesystemtype] Type '%s' already exists\n", type->Name);
        UnLockList();
        return false;
    }
    types->Prepend(type);
    UnLockList();
    return true;
}

FileSystemType *FileSystemType::GetByName(const char *name)
{
    if(!LockList()) return nullptr;
    for(auto type : *types)
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

FileSystemType *FileSystemType::GetByIndex(uint idx)
{
    if(!LockList()) return nullptr;
    auto res = types->Get(idx);
    UnLockList();
    return res;
}

void FileSystemType::Remove(FileSystemType *type)
{
    if(!LockList()) return;
    types->Remove(type, nullptr, false);
    UnLockList();
}

int FileSystemType::AutoDetect()
{
    if(!Volume::Lock()) return -EBUSY;
    int res = 0;
    for(uint i = 0;; ++i)
    {
        Volume *vol = Volume::GetByIndex(i);
        if(!vol) break;
        LockList();
        for(auto fst : *types)
        {
            if(vol->FS) continue;
            FileSystem *fs = fst->Detect(vol);
            if(!fs) continue;
            vol->FS = fs;
            FileSystem::Add(fs);
            ++res;
        }
        UnLockList();
        vol->UnLock();
    }
    Volume::UnLock();
    return res;
}

void FileSystemType::UnLockList()
{
    listLock->Release();
}

void FileSystemType::Cleanup()
{
    LockList();
    for(auto v : *types)
        delete v;
    delete listLock;
}

FileSystemType::FileSystemType(const char *name) :
    Name(strdup(name))
{
}

FileSystem *FileSystemType::Detect(Volume *vol)
{
    return nullptr;
}

FileSystemType::~FileSystemType()
{
    if(Name) free(Name);
}
