#ifndef DENTRY_H
#define DENTRY_H

#include <list.h>
#include <types.h>

class INode;

class DEntry
{
public:
    DEntry *Parent;
    List<DEntry *> *Children;
    ::INode *INode;
    int ReferenceCount;
    DEntry();
    virtual ~DEntry();
};

#endif // DENTRY_H
