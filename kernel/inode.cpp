#include <errno.h>
#include <inode.h>
#include <mutex.h>

Mutex INode::lock;

bool INode::Lock()
{
    return lock.Acquire(0, false);
}

void INode::UnLock()
{
    lock.Release();
}

INode::INode(ino_t number, FileSystem *fs) :
    Number(number), FS(fs),
    ReferenceCount(0),
    Dirty(false)
{
}

size64_t INode::GetSize()
{
    return 0;
}

mode_t INode::GetMode()
{
    return 0;
}

time_t INode::GetCreateTime()
{
    return 0;
}

time_t INode::GetModifyTime()
{
    return 0;
}

time_t INode::GetAccessTime()
{
    return 0;
}

bool INode::SetCreateTime(time_t t)
{
    return false;
}

bool INode::SetModifyTime(time_t t)
{
    return false;
}

bool INode::SetAccessTime(time_t t)
{
    return false;
}

ino_t INode::Lookup(const char *name)
{
    return -1;
}

int64_t INode::Read(void *buffer, int64_t position, int64_t n)
{
    return -ENOSYS;
}

int64_t INode::Write(const void *buffer, int64_t position, int64_t n)
{
    return -ENOSYS;
}

DirectoryEntry *INode::ReadDir(int64_t position, int64_t *newPosition)
{
    return nullptr;
}

int64_t INode::Resize(int64_t size)
{
    return -ENOSYS;
}

INode::~INode()
{
}
