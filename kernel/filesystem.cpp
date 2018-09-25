#include <filesystem.h>
#include <mutex.h>
#include <volume.h>

List<FileSystem *> *FileSystem::fileSystems;
Mutex *FileSystem::listLock;

FileSystem::FileSystem(class Volume *vol, FileSystemType *type) :
    Volume(vol),
    Type(type),
    lock(new Mutex())
{
}

FileSystem::~FileSystem()
{
    Lock();
    delete lock;
}

void FileSystem::Initialize()
{
    fileSystems = new List<FileSystem *>();
    listLock = new Mutex();
}

bool FileSystem::LockList()
{
    return listLock->Acquire(0, false);
}

void FileSystem::Add(FileSystem *fs)
{
    LockList();
    fileSystems->Append(fs);
    UnLockList();
}

FileSystem *FileSystem::GetByIndex(uint idx, bool lock)
{
    if(!LockList()) return nullptr;
    auto res = fileSystems->Get(idx);
    if(res && lock)
    {
        if(!res->Lock())
            res = nullptr;
    }
    UnLockList();
    return res;
}

void FileSystem::UnLockList()
{
    listLock->Release();
}

void FileSystem::Cleanup()
{
    LockList();
    for(auto v : *fileSystems)
    {
        if(v->Volume)
            v->Volume->FS = nullptr;
        delete v;
    }
    delete listLock;
}

bool FileSystem::Lock()
{
    return lock->Acquire(0, false);
}

void FileSystem::UnLock()
{
    return lock->Release();
}
