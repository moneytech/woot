#ifndef FILESYSTEMTYPE_H
#define FILESYSTEMTYPE_H

#include <list.h>
#include <types.h>

class FileSystem;
class Mutex;
class Volume;

class FileSystemType
{
    static List<FileSystemType *> fsTypes;
    static Mutex listLock;
protected:
    static FileSystemType *getByName_nolock(const char *name);
public:
    char *Name;

    static void Initialize();
    static bool Lock();
    static bool Add(FileSystemType *type);
    static FileSystemType *GetByName(const char *name);
    static FileSystemType *GetByIndex(uint idx);
    static void Remove(FileSystemType *type);
    static int AutoDetect();
    static void UnLock();
    static void Cleanup();

    FileSystemType(const char *name);
    virtual FileSystem *Detect(Volume *vol);
    ~FileSystemType();
};

#endif // FILESYSTEMTYPE_H
