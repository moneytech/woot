#ifndef DENTRY_H
#define DENTRY_H

#include <list.h>
#include <types.h>

class INode;
class Mutex;
class StringBuilder;

class DEntry
{
    static Mutex lock;
    static void getPath(DEntry *dentry, StringBuilder &sb);
    void getFSLabelAndID(char *buf, size_t bufSize, int *id);
public:
    DEntry *Parent;
    List<DEntry *> *Children;
    char *Name;
    ::INode *INode;
    int ReferenceCount;

    static bool Lock();
    static void UnLock();

    DEntry(const char *name, DEntry *parent);
    size_t GetFullPath(char *buffer, size_t bufferSize);
    ~DEntry();
};

#endif // DENTRY_H
