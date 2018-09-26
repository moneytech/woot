#include <inode.h>

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

INode::~INode()
{
}
