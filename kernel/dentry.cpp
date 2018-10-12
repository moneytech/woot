#include <dentry.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <stdlib.h>
#include <string.h>
#include <stringbuilder.h>

Mutex DEntry::lock;

bool DEntry::Lock()
{
    return lock.Acquire(0, false);
}

void DEntry::UnLock()
{
    lock.Release();
}

DEntry::DEntry(const char *name, DEntry *parent) :
    Parent(parent),
    Children(nullptr), // not used for now
    Name(strdup(name)),
    INode(nullptr),
    ReferenceCount(0)
{
}

DEntry::~DEntry()
{
    DEntry::Lock();
    if(Name) free(Name);
    if(INode && INode->FS) INode->FS->PutINode(INode);
    DEntry::UnLock();
}
