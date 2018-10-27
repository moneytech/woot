#include <dentry.h>
#include <file.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <stdio.h>
#include <string.h>
#include <volume.h>

Sequencer<int> FileSystem::ids(0);
List<FileSystem *> FileSystem::fileSystems;
List<INode *> FileSystem::inodeCache;
List<DEntry *> FileSystem::dentryCache;
Mutex FileSystem::lock;

FileSystem::FileSystem(class Volume *vol, FileSystemType *type) :
    ID(ids.GetNext()),
    Volume(vol),
    Type(type),
    openedFiles(new List<File *>())
{
}

FileSystem::~FileSystem()
{
    GlobalLock();
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
    GlobalLock();
    fileSystems.Append(fs);
    GlobalUnLock();
}

FileSystem *FileSystem::GetByIndex(uint idx, bool lock)
{
    if(!GlobalLock())
        return nullptr;
    FileSystem *res = fileSystems[idx];
    if(res && lock)
    {
        if(!res->GlobalLock())
            res = nullptr;
    }
    GlobalUnLock();
    return res;
}

bool FileSystem::GlobalLock()
{
    return lock.Acquire(0, false);
}

void FileSystem::GlobalUnLock()
{
    lock.Release();
}

void FileSystem::SynchronizeAll()
{
    GlobalLock();
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
    for(FileSystem *fs : fileSystems)
        fs->WriteSuperBlock();
    GlobalUnLock();
}

void FileSystem::Cleanup()
{
    GlobalLock();
    for(FileSystem *fs : fileSystems)
    {
        if(fs->Volume)
            fs->Volume->FS = nullptr;
        delete fs;
    }
    DEntry::Lock();
    for(DEntry *dentry : dentryCache)
        printf("[filesystem] WARNING: DEntry still in cache! (ref count: %d)\n", dentry->ReferenceCount);
    for(INode *inode : inodeCache)
        printf("[filesystem] WARNING: INode still in cache! (ref count: %d)\n", inode->ReferenceCount);
    DEntry::UnLock();
    GlobalUnLock();
}

bool FileSystem::GetLabel(char *buffer, size_t num)
{
    return false;
}

UUID FileSystem::GetUUID()
{
    return UUID::nil;
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

int FileSystem::GetID()
{
    return ID;
}

INode *FileSystem::GetINode(ino_t number)
{
    if(!GlobalLock())
        return nullptr;
    for(INode *inode : inodeCache)
    {
        if(inode->Number == number)
        {
            ++inode->ReferenceCount;
            GlobalUnLock();
            return inode;
        }
    }
    INode *inode = ReadINode(number);
    if(!inode)
    {
        GlobalUnLock();
        return nullptr;
    }
    inodeCache.Prepend(inode);
    ++inode->ReferenceCount;
    GlobalUnLock();
    return inode;
}

void FileSystem::PutINode(INode *inode)
{
    GlobalLock();
    if(!--inode->ReferenceCount)
    {
        inodeCache.Remove(inode, nullptr, false);
        if(inode->Dirty)
            inode->FS->WriteINode(inode);
        inode->Release();
        delete inode;
    }
    GlobalUnLock();
}

void FileSystem::SetRoot(DEntry *dentry)
{
    GlobalLock();
    if(!dentryCache.Contains(dentry, nullptr))
        dentryCache.Prepend(dentry);
    DEntry::Lock();
    ++dentry->ReferenceCount;
    Root = dentry;
    DEntry::UnLock();
    GlobalUnLock();
}

DEntry *FileSystem::GetDEntry(DEntry *parent, const char *name)
{
    if(!GlobalLock() || !parent || !parent->INode || !parent->INode->FS || !name || !DEntry::Lock())
        return nullptr;
    for(DEntry *dentry : dentryCache)
    {
        if(parent == dentry->Parent && !strcmp(name, dentry->Name))
        {
            ++dentry->ReferenceCount;
            DEntry::UnLock();
            GlobalUnLock();
            return dentry;
        }
    }
    ino_t ino = parent->INode->Lookup(name);
    if(ino <= 0)
    {
        DEntry::UnLock();
        GlobalUnLock();
        return nullptr;
    }
    DEntry *dentry = new DEntry(name, parent);
    dentry->INode = parent->INode->FS->GetINode(ino);
    dentryCache.Prepend(dentry);
    ++dentry->ReferenceCount;
    DEntry::UnLock();
    GlobalUnLock();
    return dentry;
}

DEntry *FileSystem::GetDEntry(DEntry *dentry)
{
    if(!FileSystem::GlobalLock())
        return nullptr;
    if(!DEntry::Lock())
    {
        FileSystem::GlobalUnLock();
        return nullptr;
    }
    DEntry *res = dentryCache.Find(dentry, nullptr);
    if(res) ++res->ReferenceCount;
    DEntry::UnLock();
    FileSystem::GlobalUnLock();
    return res;
}

void FileSystem::PutDEntry(DEntry *dentry)
{
    GlobalLock();
    DEntry::Lock();
    if(!--dentry->ReferenceCount)
    {
        dentryCache.Remove(dentry, nullptr, false);
        delete dentry;
    }
    DEntry::UnLock();
    GlobalUnLock();
}
