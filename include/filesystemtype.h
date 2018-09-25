#ifndef FILESYSTEMTYPE_H
#define FILESYSTEMTYPE_H

#include <list.h>
#include <types.h>

class FileSystem;
class Mutex;
class Volume;

class FileSystemType
{
    static List<FileSystemType *> *types;
    static Mutex *listLock;
protected:
public:
    char *Name;

    static void Initialize();
    static bool LockList();
    static bool Add(FileSystemType *type);
    static FileSystemType *GetByName(const char *name);
    static FileSystemType *GetByIndex(uint idx);
    static void Remove(FileSystemType *type);
    static int AutoDetect();
    static void UnLockList();
    static void Cleanup();

    FileSystemType(const char *name);
    virtual FileSystem *Detect(Volume *vol);
    ~FileSystemType();
};

#endif // FILESYSTEMTYPE_H
