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
Mutex FileSystem::lock("filesystem");

FileSystem::FileSystem(class Volume *vol, FileSystemType *type) :
    ID(ids.GetNext()),
    Volume(vol),
    Type(type),
    openedFiles(new List<File *>())
{
}

FileSystem::~FileSystem()
{
    for(File *f : *openedFiles)
        delete f;

restartdentrycache:
    for(DEntry *d : dentryCache)
    {
        if(d->INode->FS == this)
        {
            PutDEntry_nolock(d);
            goto restartdentrycache;
        }
    }

restartinodecache:
    for(INode *i : inodeCache)
    {
        if(i->FS == this)
        {
            PutINode_nolock(i);
            goto restartinodecache;
        }
    }

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
        if(!inode->Dirty)
            continue;
        inode->FS->WriteINode(inode);
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
    for(DEntry *dentry : dentryCache)
        printf("[filesystem] WARNING: DEntry still in cache! (ref count: %d)\n", dentry->ReferenceCount);
    for(INode *inode : inodeCache)
        printf("[filesystem] WARNING: INode still in cache! (ref count: %d)\n", inode->ReferenceCount);
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

INode *FileSystem::GetINode_nolock(ino_t number)
{
    for(INode *inode : inodeCache)
    {
        if(inode->FS == this && inode->Number == number)
        {
            ++inode->ReferenceCount;
            return inode;
        }
    }
    INode *inode = ReadINode(number);
    if(!inode)
        return nullptr;
    inodeCache.Prepend(inode);
    ++inode->ReferenceCount;
    return inode;
}

INode *FileSystem::GetINode(ino_t number)
{
    if(!GlobalLock())
        return nullptr;
    INode *inode = GetINode_nolock(number);
    GlobalUnLock();
    return inode;
}

void FileSystem::PutINode_nolock(INode *inode)
{
    if(!--inode->ReferenceCount)
    {
        inodeCache.Remove(inode, nullptr, false);
        if(inode->Dirty)
            inode->FS->WriteINode(inode);
        inode->Release();
        delete inode;
    }
}

void FileSystem::PutINode(INode *inode)
{
    GlobalLock();
    PutINode_nolock(inode);
    GlobalUnLock();
}

void FileSystem::SetRoot(DEntry *dentry)
{
    GlobalLock();
    if(!dentryCache.Contains(dentry, nullptr))
        dentryCache.Prepend(dentry);
    ++dentry->ReferenceCount;
    Root = dentry;
    GlobalUnLock();
}

DEntry *FileSystem::GetDEntry_nolock(DEntry *parent, const char *name)
{
    if(!parent || !parent->INode || !parent->INode->FS || !name)
        return nullptr;
    for(DEntry *dentry : dentryCache)
    {
        if(parent == dentry->Parent && !strcmp(name, dentry->Name))
        {
            ++dentry->ReferenceCount;
            return dentry;
        }
    }
    ino_t ino = parent->INode->Lookup(name);
    if(ino <= 0)
        return nullptr;
    DEntry *dentry = new DEntry(name, parent, parent->INode->FS->GetINode_nolock(ino));
    dentryCache.Prepend(dentry);
    ++dentry->ReferenceCount;
    return dentry;
}

DEntry *FileSystem::GetDEntry(DEntry *parent, const char *name)
{
    if(!GlobalLock())
        return nullptr;
    DEntry *dentry = GetDEntry_nolock(parent, name);
    GlobalUnLock();
    return dentry;
}

DEntry *FileSystem::GetDEntry_nolock(DEntry *dentry)
{
    DEntry *res = dentryCache.Find(dentry, nullptr);
    if(res) ++res->ReferenceCount;
    return res;
}

DEntry *FileSystem::GetDEntry(DEntry *dentry)
{
    if(!GlobalLock())
        return nullptr;
    DEntry *res = GetDEntry_nolock(dentry);
    GlobalUnLock();
    return res;
}

void FileSystem::PutDEntry_nolock(DEntry *dentry)
{
    if(!--dentry->ReferenceCount)
    {
        dentryCache.Remove(dentry, nullptr, false);
        delete dentry;
    }
}

void FileSystem::PutDEntry(DEntry *dentry)
{
    GlobalLock();
    PutDEntry_nolock(dentry);
    GlobalUnLock();
}
