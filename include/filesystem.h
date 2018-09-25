#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <list.h>
#include <types.h>

class FileSystemType;
class Mutex;
class Volume;

class FileSystem
{
    static List<FileSystem *> *fileSystems;
    static Mutex *listLock;
    Mutex *lock;
protected:
    FileSystem(class Volume *vol, FileSystemType *type);
    virtual ~FileSystem();
public:
    class Volume *Volume;
    FileSystemType *Type;

    static void Initialize();
    static bool LockList();
    static void Add(FileSystem *fs);
    static FileSystem *GetByIndex(uint idx, bool lock);
    static void UnLockList();
    static void Cleanup();

    bool Lock();
    void UnLock();
};

#endif // FILESYSTEM_H
