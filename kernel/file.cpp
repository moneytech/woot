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
    char *volumeSep = path[0] ? strchr(path[0], VOLUME_SEPARATOR) : nullptr;
    if(volumeSep) *volumeSep = 0;

    bool hasVolumeId = path[0] && isdigit(path[0][0]) && volumeSep;
    uint volumeId = hasVolumeId ? strtoul(path[0], nullptr, 0) : 0;
    bool hasUUID = path[0] && path[0][0] == '{' && volumeSep;
    UUID uuid = hasUUID ? UUID(path[0]) : UUID::nil;
    bool hasLabel = !hasVolumeId && !hasUUID && volumeSep;

    Volume *vol = hasLabel ? Volume::GetByLabel(path[0]) : (hasUUID ? Volume::GetByUUID(uuid) : Volume::GetByID(volumeId));
    Volume::Lock();
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
        if((hasVolumeId || hasUUID || hasLabel) && !t.Offset)
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
    if((flags & O_ACCMODE) != O_RDONLY && flags & O_TRUNC)
    {
        if(!INode::Lock())
        {
            FileSystem::UnLock();
            DEntry::UnLock();
            Volume::UnLock();
            return nullptr;
        }
        if(dentry->INode->Resize(0) != 0)
        {
            INode::UnLock();
            FileSystem::UnLock();
            DEntry::UnLock();
            Volume::UnLock();
            return nullptr;
        }
        INode::UnLock();
    }
    File *file = new File(dentry, flags);
    if(flags & O_APPEND)
        file->Position = file->GetSize();
    FileSystem::UnLock();
    DEntry::UnLock();
    Volume::UnLock();
    return file;
}

int64_t File::GetSize()
{
    if(!Lock->Acquire(0, false))
        return -EBUSY;
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
    int64_t size = DEntry && DEntry->INode ? DEntry->INode->GetSize() : -EINVAL;
    INode::UnLock();
    DEntry::UnLock();
    Lock->Release();
    return size;
}

int64_t File::Seek(int64_t offs, int loc)
{
    if(!Lock->Acquire(0, false))
        return -EBUSY;
    switch(loc)
    {
    case SEEK_SET:
        Position = offs;
        break;
    case SEEK_CUR:
        Position += offs;
        break;
    case SEEK_END:
    {
        Position = GetSize() - offs;
        break;
    }
    default:
        break;
    }
    if(Position < 0) Position = 0;
    int64_t res = Position;
    Lock->Release();
    return res;
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
    if((Flags & O_ACCMODE) == O_RDONLY)
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

int64_t File::Rewind()
{
    return Seek(0, SEEK_SET);
}

DirectoryEntry *File::ReadDir()
{
    if(!DEntry || !Lock->Acquire(0, false))
        return nullptr;
    if((Flags & O_ACCMODE) == O_WRONLY)
    {
        Lock->Release();
        return nullptr;
    }
    if(!DEntry::Lock())
    {
        Lock->Release();
        return nullptr;
    }
    if(!INode::Lock())
    {
        DEntry::UnLock();
        Lock->Release();
        return nullptr;
    }
    int64_t newPos = Position;
    DirectoryEntry *res = DEntry->INode ? DEntry->INode->ReadDir(Position, &newPos) : nullptr;
    INode::UnLock();
    if(res) Position = newPos;
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
