#include <dentry.h>
#include <filesystem.h>
#include <inode.h>
#include <mutex.h>
#include <stdlib.h>
#include <string.h>
#include <stringbuilder.h>
#include <sysdefs.h>

void DEntry::getPath(DEntry *dentry, StringBuilder &sb)
{
    if(!dentry) return;

    if(dentry->Parent)
    {
        if(dentry->Parent->Parent)
            getPath(dentry->Parent, sb);
        else
        {
            char label[MAX_FS_LABEL_LENGTH + 1] = { 0 }; int id = -1;
            dentry->getFSLabelAndID(label, MAX_FS_LABEL_LENGTH, &id);
            if(label[0]) sb.WriteFmt("%s:", label);
            else sb.WriteFmt("%d", id);
        }
        sb.WriteFmt("/%s", dentry->Name);
    }
    else
    {
        char label[MAX_FS_LABEL_LENGTH + 1] = { 0 }; int id = -1;
        dentry->getFSLabelAndID(label, MAX_FS_LABEL_LENGTH, &id);
        if(label[0]) sb.WriteFmt("%s:%s", label, dentry->Name);
        else sb.WriteFmt("%d:%s", id, dentry->Name);
    }
}

void DEntry::getFSLabelAndID(char *buf, size_t bufSize, int *id)
{
    if(INode->FS)
    {
        INode->FS->GetLabel(buf, bufSize);
        if(id) *id = INode->FS->GetID();
    }
}

DEntry::DEntry(const char *name, DEntry *parent) :
    Parent(parent),
    Children(nullptr), // not used for now
    Name(strdup(name)),
    INode(nullptr),
    ReferenceCount(0)
{
}

size_t DEntry::GetFullPath(char *buffer, size_t bufferSize)
{
    StringBuilder sb(buffer, bufferSize);
    getPath(this, sb);
    return strlen(buffer);
}

DEntry::~DEntry()
{
    if(Name) free(Name);
    FileSystem::PutINode_nolock(INode);
    if(Parent) FileSystem::PutDEntry_nolock(Parent);
}
