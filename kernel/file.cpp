#include <dentry.h>
#include <errno.h>
#include <file.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>

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

int64_t File::Read(void *buffer, int64_t n)
{
    if(!DEntry || !Lock->Acquire(0, false))
        return -EBUSY;
    if((Flags & O_ACCMODE) == O_WRONLY)
    {
        Lock->Release();
        return -EINVAL;
    }
    if(!DEntry->Lock->Acquire(0, false))
    {
        Lock->Release();
        return -EBUSY;
    }
    int64_t res = DEntry->INode->Read(buffer, Position, n);
    if(res > 0) Position += res;
    DEntry->Lock->Release();
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
    if(!DEntry->Lock->Acquire(0, false))
    {
        Lock->Release();
        return -EBUSY;
    }
    int64_t res = DEntry->INode->Write(buffer, Position, n);
    if(res > 0) Position += res;
    DEntry->Lock->Release();
    Lock->Release();
    return res;
}

File::~File()
{
    Lock->Acquire(0, false);
    if(DEntry)
    {
        DEntry->Lock->Acquire(0, false);
        FileSystem *FS = DEntry->INode->FS;
        FS->Lock();
        bool deleted = FS->PutDEntry(DEntry);
        FS->UnLock();
        if(!deleted) DEntry->Lock->Release();
    }
    delete Lock;
}
