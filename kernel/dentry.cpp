#include <dentry.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <stdlib.h>
#include <string.h>

DEntry::DEntry(const char *name, DEntry *parent) :
    Parent(parent),
    Children(nullptr),
    Name(strdup(name)),
    INode(nullptr),
    ReferenceCount(0),
    Lock(new Mutex())
{

}

DEntry::~DEntry()
{
    Lock->Acquire(0, false);
    if(Name) free(Name);
    if(INode && INode->FS) INode->FS->PutINode(INode);
    delete Lock;
}
