#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <volume.h>

List<FileSystem *> *FileSystem::fileSystems;
Mutex *FileSystem::listLock;

FileSystem::FileSystem(class Volume *vol, FileSystemType *type) :
    Volume(vol),
    Type(type),
    lock(new Mutex()),
    inodeCache(new List<INode *>())
{
}

FileSystem::~FileSystem()
{
    Lock();
    delete inodeCache;
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

INode *FileSystem::ReadINode(ino_t number)
{
    return nullptr;
}

bool FileSystem::WriteINode(INode *inode)
{
    return false;
}

bool FileSystem::WriteSuperBlock()
{
    return false;
}

INode *FileSystem::GetINode(ino_t number)
{
    Lock();
    for(INode *inode : *inodeCache)
    {
        if(inode->Number == number)
        {
            ++inode->ReferenceCount;
            UnLock();
            return inode;
        }
    }
    INode *inode = ReadINode(number);
    if(!inode)
    {
        UnLock();
        return nullptr;
    }
    inodeCache->Prepend(inode);
    ++inode->ReferenceCount;
    UnLock();
    return inode;
}

void FileSystem::PutINode(INode *inode)
{
    Lock();
    if(!--inode->ReferenceCount)
    {
        inodeCache->Remove(inode, nullptr, false);
        if(inode->Dirty)
            WriteINode(inode);
        delete inode;
    }
    UnLock();
}

void FileSystem::UnLock()
{
    return lock->Release();
}
