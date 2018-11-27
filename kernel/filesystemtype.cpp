#include <errno.h>
#include <filesystem.h>
#include <filesystemtype.h>
#include <mutex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volume.h>

List<FileSystemType *> FileSystemType::fsTypes;
Mutex FileSystemType::listLock("fsList");

FileSystemType *FileSystemType::getByName_nolock(const char *name)
{
    for(FileSystemType *type : fsTypes)
    {
        if(!strcmp(name, type->Name))
            return type;
    }
    return nullptr;
}

void FileSystemType::Initialize()
{
}

bool FileSystemType::Lock()
{
    return listLock.Acquire(0, false);
}

bool FileSystemType::Add(FileSystemType *type)
{
    if(!Lock()) return false;
    FileSystemType *t = getByName_nolock(type->Name);
    if(t)
    {
        printf("[filesystemtype] Type '%s' already exists\n", type->Name);
        UnLock();
        return false;
    }
    fsTypes.Prepend(type);
    UnLock();
    return true;
}

FileSystemType *FileSystemType::GetByName(const char *name)
{
    if(!Lock()) return nullptr;
    FileSystemType *res = getByName_nolock(name);
    UnLock();
    return res;
}

FileSystemType *FileSystemType::GetByIndex(uint idx)
{
    if(!Lock()) return nullptr;
    FileSystemType *res = fsTypes[idx];
    UnLock();
    return res;
}

void FileSystemType::Remove(FileSystemType *type)
{
    if(!Lock()) return;
    fsTypes.Remove(type, nullptr, false);
    UnLock();
}

int FileSystemType::AutoDetect()
{
    int res = 0;
    for(uint i = 0;; ++i)
    {
        Volume *vol = Volume::GetByIndex_nolock(i);
        if(!vol) break;
        Lock();
        for(FileSystemType *fst : fsTypes)
        {
            if(vol->FS) continue;
            FileSystem *fs = fst->Detect(vol);
            if(!fs) continue;
            vol->FS = fs;
            FileSystem::Add(fs);
            ++res;
        }
        UnLock();
    }
    return res;
}

void FileSystemType::UnLock()
{
    listLock.Release();
}

void FileSystemType::Cleanup()
{
    Lock();
    for(FileSystemType *fst : fsTypes)
        delete fst;
    UnLock();
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
