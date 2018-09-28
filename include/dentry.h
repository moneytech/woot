#ifndef DENTRY_H
#define DENTRY_H

#include <list.h>
#include <types.h>

class INode;
class Mutex;

class DEntry
{
public:
    DEntry *Parent;
    List<DEntry *> *Children;
    char *Name;
    ::INode *INode;
    int ReferenceCount;
    Mutex *Lock;

    DEntry(const char *name, DEntry *parent);
    ~DEntry();
};

#endif // DENTRY_H
