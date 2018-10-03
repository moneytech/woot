#include <directoryentry.h>
#include <stdlib.h>
#include <string.h>

DirectoryEntry::DirectoryEntry(mode_t mode, time_t atime, time_t ctime, time_t mtime, size64_t size, ino_t inode, const char *name) :
    Mode(mode),
    AccessTime(atime),
    CreateTime(ctime),
    ModifyTime(mtime),
    Size(size),
    INode(inode),
    Name(strdup(name))
{
}

DirectoryEntry::~DirectoryEntry()
{
    if(Name) free(Name);
}
