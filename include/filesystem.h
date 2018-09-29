#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <list.h>
#include <types.h>

class DEntry;
class File;
class FileSystemType;
class INode;
class Mutex;
class Volume;

class FileSystem
{
    static List<FileSystem *> fileSystems;
    static List<INode *> inodeCache;
    static List<DEntry *> dentryCache;
    static Mutex lock;

    List<File *> *openedFiles;
protected:
    FileSystem(class Volume *vol, FileSystemType *type);
    virtual ~FileSystem();
public:
    class Volume *Volume;
    FileSystemType *Type;
    DEntry *Root;

    static void Initialize();
    static void Add(FileSystem *fs);
    static FileSystem *GetByIndex(uint idx, bool lock);
    static bool Lock();
    static void UnLock();
    static void SynchronizeAll();
    static void Cleanup();

    virtual INode *ReadINode(ino_t number);
    virtual bool WriteINode(INode *inode);
    virtual bool WriteSuperBlock();

    INode *GetINode(ino_t number);
    void PutINode(INode *inode);
    void SetRoot(DEntry *dentry);
    DEntry *GetDEntry(DEntry *parent, const char *name);
    void PutDEntry(DEntry *dentry);
};

#endif // FILESYSTEM_H
