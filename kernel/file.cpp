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

#include <stdio.h>

File::File(::DEntry *dentry, int flags, mode_t mode) :
    DEntry(dentry),
    Flags(flags),
    Position(0),
    Mode(mode)
{
}

File *File::Open_nolock(::DEntry *parent, const char *name, int flags)
{

    //printf("open %s\n", name);
    if(!parent || !parent->INode || !name)
        return nullptr;

    Tokenizer path(name, PATH_SEPARATORS, 0);

    ::DEntry *dentry = FileSystem::GetDEntry_nolock(parent);
    for(Tokenizer::Token t : path.Tokens)
    {
        if(!strcmp(".", t.String))
            continue;
        else if(!strcmp("..", t.String))
        {
            if(dentry->Parent)
            {
                ::DEntry *nextDe = FileSystem::GetDEntry_nolock(dentry->Parent);
                FileSystem::PutDEntry_nolock(dentry);
                dentry = nextDe;
            }
            continue;
        }
        ::DEntry *nextDe = FileSystem::GetDEntry_nolock(dentry, t.String);
        if(!nextDe)
        {
            FileSystem::PutDEntry_nolock(dentry);
            return nullptr;
        }
        dentry = nextDe;
    }
    if((flags & O_ACCMODE) != O_RDONLY && flags & O_TRUNC)
    {
        if(dentry->INode->Resize(0) != 0)
        {
            FileSystem::PutDEntry_nolock(dentry);
            return nullptr;
        }
    }
    mode_t mode = dentry->INode->GetMode();
    if((flags & O_DIRECTORY && !S_ISDIR(mode)))
    {
        FileSystem::PutDEntry_nolock(dentry);
        return nullptr;
    }
    File *file = new File(dentry, flags, mode);
    if(flags & O_APPEND)
        file->Position = file->GetSize();
    return file;
}

File *File::Open(::DEntry *parent, const char *name, int flags)
{
    //printf("open %s\n", name);
    if(!FileSystem::GlobalLock())
        return nullptr;
    File *file = Open_nolock(parent, name, flags);
    FileSystem::GlobalUnLock();
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
    Volume *vol = hasLabel ? Volume::GetByLabel_nolock(path[0]) : (hasUUID ? Volume::GetByUUID(uuid) : Volume::GetByID_nolock(volumeId));
    if(!vol || !vol->FS)
    {
        Volume::UnLock();
        return nullptr;
    }
    if(!FileSystem::GlobalLock())
    {
        Volume::UnLock();
        return nullptr;
    }
    bool hasVolume = hasVolumeId || hasUUID || hasLabel;
    bool absolute = !hasVolume && path[0][0] == '/';
    ::DEntry *dentry = hasVolume || absolute ? vol->FS->Root : Process::GetCurrentDir();
    Volume::UnLock();

    if(!dentry)
    {
        FileSystem::GlobalUnLock();
        return nullptr;
    }

    File *file = Open_nolock(dentry, name + (hasVolume ? path.Tokens[1].Offset : 0), flags);

    FileSystem::GlobalUnLock();
    return file;
}

int64_t File::GetSize_nolock()
{
    int64_t size = DEntry && DEntry->INode ? DEntry->INode->GetSize() : -EINVAL;
    return size;
}

int64_t File::GetSize()
{
    if(!FileSystem::GlobalLock())
        return -EBUSY;
    int64_t size = GetSize_nolock();
    FileSystem::GlobalUnLock();
    return size;
}

bool File::SetAccessTime(time_t time)
{
    if(!FileSystem::GlobalLock())
        return -EBUSY;
    bool res = DEntry && DEntry->INode ? DEntry->INode->SetAccessTime(time) : -EINVAL;
    FileSystem::GlobalUnLock();
    return res;
}

bool File::SetModifyTime(time_t time)
{
    if(!FileSystem::GlobalLock())
        return -EBUSY;
    bool res = DEntry && DEntry->INode ? DEntry->INode->SetModifyTime(time) : -EINVAL;
    FileSystem::GlobalUnLock();
    return res;
}

bool File::Create(const char *name, mode_t mode)
{
    if(!FileSystem::GlobalLock())
        return false;
    bool res = DEntry && DEntry->INode ? DEntry->INode->Create(name, mode) : false;
    FileSystem::GlobalUnLock();
    return res;
}

int File::Remove(const char *name)
{
    if(!FileSystem::GlobalLock())
        return false;
    int res = DEntry && DEntry->INode ? DEntry->INode->Remove(name) : -EINVAL;
    FileSystem::GlobalUnLock();
    return res;
}

int64_t File::Seek(int64_t offs, int loc)
{
    if(S_ISDIR(Mode) && (offs != 0 || loc != SEEK_SET))
        return -EISDIR;
    switch(loc)
    {
    case SEEK_SET:
        Position = offs;
        break;
    case SEEK_CUR:
        Position += offs;
        break;
    case SEEK_END:
        Position = GetSize_nolock() - offs;
        break;
    default:
        break;
    }
    if(Position < 0) Position = 0;
    int64_t res = Position;
    return res;
}

int64_t File::Read(void *buffer, int64_t n)
{    
    if(!DEntry) return -EINVAL;
    if(S_ISDIR(Mode))
        return -EISDIR;
    if((Flags & O_ACCMODE) == O_WRONLY)
        return -EINVAL;
    if(!FileSystem::GlobalLock())
        return -EBUSY;
    int64_t res = DEntry->INode ? DEntry->INode->Read(buffer, Position, n) : -EINVAL;
    if(res > 0) Position += res;
    FileSystem::GlobalUnLock();
    return res;
}

int64_t File::Write(const void *buffer, int64_t n)
{
    if(!DEntry) return -EINVAL;
    if(S_ISDIR(Mode))
        return -EISDIR;
    if((Flags & O_ACCMODE) == O_RDONLY)
        return -EINVAL;
    if(!FileSystem::GlobalLock())
        return -EBUSY;
    int64_t res = DEntry->INode ? DEntry->INode->Write(buffer, Position, n) : -EINVAL;
    if(res > 0) Position += res;
    FileSystem::GlobalUnLock();
    return res;
}

int64_t File::Rewind()
{
    return Seek(0, SEEK_SET);
}

DirectoryEntry *File::ReadDir()
{
    if(!DEntry)
        return nullptr;
    if(!(S_ISDIR(Mode)))
    {   // not a directory
        return nullptr;
    }
    if((Flags & O_ACCMODE) == O_WRONLY)
        return nullptr;
    if(!FileSystem::GlobalLock())
        return nullptr;
    int64_t newPos = Position;
    DirectoryEntry *res = DEntry->INode ? DEntry->INode->ReadDir(Position, &newPos) : nullptr;
    if(res) Position = newPos;
    FileSystem::GlobalUnLock();
    return res;
}

File::~File()
{
    //printf("close\n");
    FileSystem::PutDEntry(DEntry);
}
