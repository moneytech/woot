#include <dentry.h>
#include <errno.h>
#include <file.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <process.h>
#include <stat.h>
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
    if(!DEntry::Lock())
        return nullptr;

    if(!parent || !parent->INode || !name)
    {
        DEntry::UnLock();
        return nullptr;
    }

    Tokenizer path(name, PATH_SEPARATORS, 0);

    ::DEntry *dentry = FileSystem::GetDEntry(parent);
    for(Tokenizer::Token t : path.Tokens)
    {
        if(!strcmp(".", t.String))
            continue;
        else if(!strcmp("..", t.String))
        {
            if(dentry->Parent)
            {
                ::DEntry *nextDe = FileSystem::GetDEntry(dentry->Parent);
                FileSystem::PutDEntry(dentry);
                dentry = nextDe;
            }
            continue;
        }
        ::DEntry *nextDe = FileSystem::GetDEntry(dentry, t.String);
        if(!nextDe)
        {
            FileSystem::PutDEntry(dentry);
            DEntry::UnLock();
            return nullptr;
        }
        dentry = nextDe;
    }
    if((flags & O_ACCMODE) != O_RDONLY && flags & O_TRUNC)
    {
        if(!INode::Lock())
        {
            FileSystem::PutDEntry(dentry);
            DEntry::UnLock();
            return nullptr;
        }
        if(dentry->INode->Resize(0) != 0)
        {
            FileSystem::PutDEntry(dentry);
            INode::UnLock();
            DEntry::UnLock();
            return nullptr;
        }
        INode::UnLock();
    }
    mode_t mode = dentry->INode->GetMode();
    if((flags & O_DIRECTORY && !S_ISDIR(mode)) || (!(flags & O_DIRECTORY) && S_ISDIR(mode)))
    {
        FileSystem::PutDEntry(dentry);
        return nullptr;
    }
    File *file = new File(dentry, flags);
    if(flags & O_APPEND)
        file->Position = file->GetSize();
    DEntry::UnLock();
    return file;
}

File *File::Open(const char *name, int flags)
{
    if(!name || !strlen(name))
        name = ".";
    Tokenizer path(name, PATH_SEPARATORS, 0);
    if(!path[0]) return nullptr;
    char *volumeSep = path[0] ? strchr(path[0], VOLUME_SEPARATOR) : nullptr;
    if(volumeSep) *volumeSep = 0;

    bool hasVolumeId = path[0] && isdigit(path[0][0]) && volumeSep;
    uint volumeId = hasVolumeId ? strtoul(path[0], nullptr, 0) : 0;
    bool hasUUID = path[0] && path[0][0] == '{' && volumeSep;
    UUID uuid = hasUUID ? UUID(path[0]) : UUID::nil;
    bool hasLabel = !hasVolumeId && !hasUUID && volumeSep;

    if(!Volume::Lock())
        return nullptr;
    Volume *vol = hasLabel ? Volume::GetByLabel(path[0]) : (hasUUID ? Volume::GetByUUID(uuid) : Volume::GetByID(volumeId));
    if(!vol || !vol->FS)
    {
        Volume::UnLock();
        return nullptr;
    }
    if(!DEntry::Lock())
    {
        Volume::UnLock();
        return nullptr;
    };
    if(!FileSystem::GlobalLock())
    {
        Volume::UnLock();
        DEntry::UnLock();
        return nullptr;
    }
    bool hasVolume = hasVolumeId || hasUUID || hasLabel;
    bool absolute = !hasVolume && path[0][0] == '/';
    ::DEntry *dentry = hasVolume || absolute ? vol->FS->Root : Process::GetCurrentDir();
    FileSystem::GlobalUnLock();
    Volume::UnLock();

    if(!dentry)
    {
        DEntry::UnLock();
        return nullptr;
    }

    File *file = Open(dentry, name + (hasVolume ? path.Tokens[1].Offset : 0), flags);

    DEntry::UnLock();
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
    int64_t size = DEntry && DEntry->INode ? DEntry->INode->GetSize() : -EINVAL;
    DEntry::UnLock();
    Lock->Release();
    return size;
}

bool File::SetAccessTime(time_t time)
{
    if(!Lock->Acquire(0, false))
        return -EBUSY;
    if(!DEntry::Lock())
    {
        Lock->Release();
        return -EBUSY;
    }
    bool res = DEntry && DEntry->INode ? DEntry->INode->SetAccessTime(time) : -EINVAL;
    DEntry::UnLock();
    Lock->Release();
    return res;
}

bool File::SetModifyTime(time_t time)
{
    if(!Lock->Acquire(0, false))
        return -EBUSY;
    if(!DEntry::Lock())
    {
        Lock->Release();
        return -EBUSY;
    }
    bool res = DEntry && DEntry->INode ? DEntry->INode->SetModifyTime(time) : -EINVAL;
    DEntry::UnLock();
    Lock->Release();
    return res;
}

bool File::Create(const char *name, mode_t mode)
{
    if(!Lock->Acquire(0, false))
        return false;
    if(!DEntry::Lock())
    {
        Lock->Release();
        return false;
    }
    bool res = DEntry && DEntry->INode ? DEntry->INode->Create(name, mode) : false;
    DEntry::UnLock();
    Lock->Release();
    return res;
}

int File::Remove(const char *name)
{
    if(!Lock->Acquire(0, false))
        return false;
    if(!DEntry::Lock())
    {
        Lock->Release();
        return false;
    }
    int res = DEntry && DEntry->INode ? DEntry->INode->Remove(name) : -EINVAL;
    DEntry::UnLock();
    Lock->Release();
    return res;
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
        Position = GetSize() - offs;
        break;
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
    FileSystem::PutDEntry(DEntry);
    delete Lock;
}
