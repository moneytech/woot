#include <dentry.h>
#include <file.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <string.h>
#include <volume.h>

List<FileSystem *> *FileSystem::fileSystems;
Mutex *FileSystem::listLock;

FileSystem::FileSystem(class Volume *vol, FileSystemType *type) :
    Volume(vol),
    Type(type),
    lock(new Mutex()),
    inodeCache(new List<INode *>()),
    dentryCache(new List<DEntry *>()),
    openedFiles(new List<File *>())
{
}

FileSystem::~FileSystem()
{
    Lock();
    for(File *f : *openedFiles)
        delete f;
    for(DEntry *d : *dentryCache)
        delete d;
    for(INode *i : *inodeCache)
        delete i;
    delete inodeCache;
    delete dentryCache;
    delete openedFiles;
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
    if(!Lock()) return nullptr;
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

void FileSystem::SetRoot(DEntry *dentry)
{
    Lock();
    if(!dentryCache->Contains(dentry, nullptr))
        dentryCache->Prepend(dentry);
    dentry->Lock->Acquire(0, false);
    ++dentry->ReferenceCount;
    Root = dentry;
    dentry->Lock->Release();
    UnLock();
}

DEntry *FileSystem::GetDEntry(DEntry *parent, const char *name)
{
    if(!Lock() || !parent || !parent->INode || !name || !parent->Lock->Acquire(0, false))
        return nullptr;
    for(DEntry *dentry : *dentryCache)
    {
        if(!dentry->Lock->Acquire(0, false))
        {
            parent->Lock->Release();
            UnLock();
            return nullptr;
        }
        if(parent == dentry->Parent && !strcmp(name, dentry->Name))
        {
            ++dentry->ReferenceCount;
            dentry->Lock->Release();
            parent->Lock->Release();
            UnLock();
            return dentry;
        }
        dentry->Lock->Release();
    }
    ino_t ino = parent->INode->Lookup(name);
    if(ino < 0)
    {
        parent->Lock->Release();
        UnLock();
        return nullptr;
    }
    DEntry *dentry = new DEntry(name, parent);
    if(!dentry->Lock->Acquire(0, false))
    {
        delete dentry;
        parent->Lock->Release();
        UnLock();
        return nullptr;
    }
    dentry->INode = GetINode(ino);
    dentryCache->Prepend(dentry);
    ++dentry->ReferenceCount;
    dentry->Lock->Release();
    parent->Lock->Release();
    UnLock();
    return dentry;
}

bool FileSystem::PutDEntry(DEntry *dentry)
{
    Lock();
    dentry->Lock->Acquire(0, false);
    if(!--dentry->ReferenceCount)
    {
        dentryCache->Remove(dentry, nullptr, false);
        delete dentry;
        UnLock();
        return true;
    }
    dentry->Lock->Release();
    UnLock();
    return false;
}

void FileSystem::UnLock()
{
    return lock->Release();
}
