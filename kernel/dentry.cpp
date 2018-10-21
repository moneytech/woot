#include <dentry.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <stdlib.h>
#include <string.h>
#include <stringbuilder.h>

Mutex DEntry::lock;

void DEntry::getPath(DEntry *dentry, StringBuilder &sb)
{
    if(dentry->Parent)
    {
        if(dentry->Parent->Parent)
            getPath(dentry->Parent, sb);
        sb.WriteFmt("/%s", dentry->Name);
    } else sb.WriteFmt("%s", dentry->Name);;
}

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

bool DEntry::GetFullPath(char *buffer, size_t bufferSize)
{
    if(!Lock())
        return false;
    StringBuilder sb(buffer, bufferSize);
    getPath(this, sb);
    UnLock();
    return true;
}

DEntry::~DEntry()
{
    DEntry::Lock();
    if(Name) free(Name);
    FileSystem::PutINode(INode);
    if(Parent) FileSystem::PutDEntry(Parent);
    DEntry::UnLock();
}
