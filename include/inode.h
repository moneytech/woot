#ifndef INODE_H
#define INODE_H

#include <types.h>

class FileSystem;

class INode
{
public:
    ino_t Number;
    FileSystem *FS;
    int ReferenceCount;
    bool Dirty;
    INode(ino_t number, FileSystem *fs);
    virtual size64_t GetSize();
    virtual time_t GetCreateTime();
    virtual time_t GetModifyTime();
    virtual time_t GetAccessTime();
    virtual ~INode();
};

#endif // INODE_H
