#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <list.h>
#include <types.h>

class FileSystemType;
class INode;
class Mutex;
class Volume;

class FileSystem
{
    static List<FileSystem *> *fileSystems;
    static Mutex *listLock;
    Mutex *lock;
    List<INode *> *inodeCache;
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
    virtual INode *ReadINode(ino_t number);
    virtual bool WriteINode(INode *inode);
    virtual bool WriteSuperBlock();
    INode *GetINode(ino_t number);
    void PutINode(INode *inode);
    void UnLock();
};

#endif // FILESYSTEM_H
