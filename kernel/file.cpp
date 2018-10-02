#include <dentry.h>
#include <errno.h>
#include <file.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <string.h>
#include <sysdefs.h>
#include <tokenizer.h>
#include <volume.h>

File::File(::DEntry *dentry, int flags) :
    DEntry(dentry),
    Flags(flags),
    Position(0),
    Lock(new Mutex())
{
}

File *File::Open(::DEntry *parent, const char *name, int flags)
{
    if(!parent || !parent->INode || !parent->INode->FS || !name)
        return nullptr;
    FileSystem *FS = parent->INode->FS;
    if(!FS->Lock())
        return nullptr;
    ::DEntry *dentry = FS->GetDEntry(parent, name);
    if(!dentry)
    {
        FS->UnLock();
        return nullptr;
    }
    File *file = new File(dentry, flags);
    FS->UnLock();
    return file;
}

File *File::Open(const char *name, int flags)
{
    Tokenizer path(name, PATH_SEPARATORS, 0);
    bool hasVolumeId = path[0] && strchr(path[0], VOLUME_SEPARATOR);
    uint volumeId = hasVolumeId ? strtoul(path[0], nullptr, 0) : 0;
    Volume::Lock();
    Volume *vol = Volume::GetByID(volumeId);
    if(!vol || !vol->FS)
    {
        Volume::UnLock();
        return nullptr;
    }
    DEntry::Lock();
    FileSystem::Lock();
    ::DEntry *dentry = FileSystem::GetDEntry(vol->FS->Root);
    if(!dentry)
    {
        FileSystem::UnLock();
        DEntry::UnLock();
        Volume::UnLock();
        return nullptr;
    }
    for(Tokenizer::Token t : path.Tokens)
    {
        if(hasVolumeId && !t.Offset)
            continue;
        ::DEntry *nextDe = FileSystem::GetDEntry(dentry, t.String);
        FileSystem::PutDEntry(dentry);
        if(!nextDe)
        {
            FileSystem::UnLock();
            DEntry::UnLock();
            Volume::UnLock();
            return nullptr;
        }
        dentry = nextDe;
    }
    File *file = new File(dentry, flags);
    FileSystem::UnLock();
    DEntry::UnLock();
    Volume::UnLock();
    return file;
}

int64_t File::Read(void *buffer, int64_t n)
{
    if(!DEntry || !Lock->Acquire(0, false))
        return -EBUSY;
    if((Flags & O_ACCMODE) == O_WRONLY)
    {
        Lock->Release();
        return -EINVAL;
    }
    if(!DEntry::Lock())
    {
        Lock->Release();
        return -EBUSY;
    }
    if(!INode::Lock())
    {
        DEntry::UnLock();
        Lock->Release();
        return -EBUSY;
    }
    int64_t res = DEntry->INode ? DEntry->INode->Read(buffer, Position, n) : -EINVAL;
    INode::UnLock();
    if(res > 0) Position += res;
    DEntry::UnLock();
    Lock->Release();
    return res;
}

int64_t File::Write(const void *buffer, int64_t n)
{
    if(!DEntry || !Lock->Acquire(0, false))
        return -EBUSY;
    if((Flags & O_ACCMODE) == O_WRONLY)
    {
        Lock->Release();
        return -EINVAL;
    }
    if(!DEntry::Lock())
    {
        Lock->Release();
        return -EBUSY;
    }
    if(!INode::Lock())
    {
        DEntry::UnLock();
        Lock->Release();
        return -EBUSY;
    }
    int64_t res = DEntry->INode ? DEntry->INode->Write(buffer, Position, n) : -EINVAL;
    INode::UnLock();
    if(res > 0) Position += res;
    DEntry::UnLock();
    Lock->Release();
    return res;
}

File::~File()
{
    Lock->Acquire(0, false);
    DEntry::Lock();
    INode::Lock();
    DEntry->INode->FS->PutDEntry(DEntry);
    INode::UnLock();
    DEntry::UnLock();
    delete Lock;
}
