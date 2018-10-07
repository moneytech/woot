#ifndef INODE_H
#define INODE_H

#include <list.h>
#include <types.h>

class DEntry;
class DirectoryEntry;
class FileSystem;
class Mutex;

class INode
{
    static Mutex lock;
public:
    ino_t Number;
    FileSystem *FS;
    int ReferenceCount;
    bool Dirty;
    //List<DEntry *> DEntries;

    static bool Lock();
    static void UnLock();

    INode(ino_t number, FileSystem *fs);
    virtual size64_t GetSize();
    virtual mode_t GetMode();
    virtual time_t GetCreateTime();
    virtual time_t GetModifyTime();
    virtual time_t GetAccessTime();
    virtual bool SetCreateTime(time_t t);
    virtual bool SetModifyTime(time_t t);
    virtual bool SetAccessTime(time_t t);
    virtual ino_t Lookup(const char *name);
    virtual int64_t Read(void *buffer, int64_t position, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t position, int64_t n);
    virtual DirectoryEntry *ReadDir(int64_t position, int64_t *newPosition);
    virtual int64_t Resize(int64_t size);
    virtual ~INode();
};

#endif // INODE_H
