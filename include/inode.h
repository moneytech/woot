#ifndef INODE_H
#define INODE_H

#include <types.h>

// TODO: Mutexify

class DEntry;
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
    virtual ino_t Lookup(const char *name);
    virtual int64_t Read(void *buffer, int64_t position, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t position, int64_t n);
    virtual ~INode();
};

#endif // INODE_H
