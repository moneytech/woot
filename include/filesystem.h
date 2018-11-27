#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <list.h>
#include <sequencer.h>
#include <types.h>
#include <uuid.h>

class DEntry;
class File;
class FileSystemType;
class INode;
class Mutex;
class Volume;

class FileSystem
{
    static Sequencer<int> ids;
    static List<FileSystem *> fileSystems;
    static List<INode *> inodeCache;
    static List<DEntry *> dentryCache;
    static Mutex lock;

    List<File *> *openedFiles;
protected:
    int ID;
    FileSystem(class Volume *vol, FileSystemType *type);
    virtual ~FileSystem();
public:
    class Volume *Volume;
    FileSystemType *Type;
    DEntry *Root;

    static void Initialize();
    static void Add(FileSystem *fs);
    static FileSystem *GetByIndex(uint idx, bool lock);
    static bool GlobalLock();
    static void GlobalUnLock();
    static void SynchronizeAll();
    static void Cleanup();

    virtual bool GetLabel(char *buffer, size_t num);
    virtual UUID GetUUID();
    virtual INode *ReadINode(ino_t number);
    virtual bool WriteINode(INode *inode);
    virtual bool WriteSuperBlock();

    int GetID();
    INode *GetINode_nolock(ino_t number);
    INode *GetINode(ino_t number);
    static void PutINode_nolock(INode *inode);
    static void PutINode(INode *inode);
    void SetRoot(DEntry *dentry);
    static DEntry *GetDEntry_nolock(DEntry *parent, const char *name);
    static DEntry *GetDEntry(DEntry *parent, const char *name);
    static DEntry *GetDEntry_nolock(DEntry *dentry);
    static DEntry *GetDEntry(DEntry *dentry); // used to make reference counter (great again)
    static void PutDEntry_nolock(DEntry *dentry);
    static void PutDEntry(DEntry *dentry);
};

#endif // FILESYSTEM_H
