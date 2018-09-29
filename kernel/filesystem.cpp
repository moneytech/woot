#include <dentry.h>
#include <file.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <string.h>
#include <volume.h>

List<FileSystem *> FileSystem::fileSystems;
List<INode *> FileSystem::inodeCache;
List<DEntry *> FileSystem::dentryCache;
Mutex FileSystem::lock;

FileSystem::FileSystem(class Volume *vol, FileSystemType *type) :
    Volume(vol),
    Type(type),
    openedFiles(new List<File *>())
{
}

FileSystem::~FileSystem()
{
    Lock();
    for(File *f : *openedFiles)
        delete f;
    for(DEntry *d : dentryCache)
        delete d;
    for(INode *i : inodeCache)
        delete i;
    delete openedFiles;
}

void FileSystem::Initialize()
{
}

void FileSystem::Add(FileSystem *fs)
{
    Lock();
    fileSystems.Append(fs);
    UnLock();
}

FileSystem *FileSystem::GetByIndex(uint idx, bool lock)
{
    if(!Lock())
        return nullptr;
    FileSystem *res = fileSystems[idx];
    if(res && lock)
    {
        if(!res->Lock())
            res = nullptr;
    }
    UnLock();
    return res;
}

bool FileSystem::Lock()
{
    return lock.Acquire(0, false);
}

void FileSystem::UnLock()
{
    lock.Release();
}

void FileSystem::SynchronizeAll()
{
    Lock();
    for(INode *inode : inodeCache)
    {
        INode::Lock();
        if(!inode->Dirty)
        {
            INode::UnLock();
            continue;
        }
        inode->FS->WriteINode(inode);
        INode::UnLock();
    }
    UnLock();
}

void FileSystem::Cleanup()
{
    Lock();
    for(FileSystem *fs : fileSystems)
    {
        if(fs->Volume)
            fs->Volume->FS = nullptr;
        delete fs;
    }
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
    if(!Lock())
        return nullptr;
    for(INode *inode : inodeCache)
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
    inodeCache.Prepend(inode);
    ++inode->ReferenceCount;
    UnLock();
    return inode;
}

void FileSystem::PutINode(INode *inode)
{
    Lock();
    if(!--inode->ReferenceCount)
    {
        inodeCache.Remove(inode, nullptr, false);
        if(inode->Dirty)
            WriteINode(inode);
        delete inode;
    }
    UnLock();
}

void FileSystem::SetRoot(DEntry *dentry)
{
    Lock();
    if(!dentryCache.Contains(dentry, nullptr))
        dentryCache.Prepend(dentry);
    DEntry::Lock();
    ++dentry->ReferenceCount;
    Root = dentry;
    DEntry::UnLock();
    UnLock();
}

DEntry *FileSystem::GetDEntry(DEntry *parent, const char *name)
{
    if(!Lock() || !parent || !parent->INode || !name || !DEntry::Lock())
        return nullptr;
    for(DEntry *dentry : dentryCache)
    {
        if(parent == dentry->Parent && !strcmp(name, dentry->Name))
        {
            ++dentry->ReferenceCount;
            DEntry::UnLock();
            UnLock();
            return dentry;
        }
    }
    ino_t ino = parent->INode->Lookup(name);
    if(ino < 0)
    {
        DEntry::UnLock();
        UnLock();
        return nullptr;
    }
    DEntry *dentry = new DEntry(name, parent);
    dentry->INode = GetINode(ino);
    dentryCache.Prepend(dentry);
    ++dentry->ReferenceCount;
    DEntry::UnLock();
    UnLock();
    return dentry;
}

void FileSystem::PutDEntry(DEntry *dentry)
{
    Lock();
    DEntry::Lock();
    if(!--dentry->ReferenceCount)
    {
        dentryCache.Remove(dentry, nullptr, false);
        delete dentry;
    }
    DEntry::UnLock();
    UnLock();
}
