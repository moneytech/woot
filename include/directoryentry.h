#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H

#include <types.h>

// general class for directory listing
class DirectoryEntry
{
public:
    mode_t Mode;
    time_t AccessTime;
    time_t CreateTime;
    time_t ModifyTime;
    size64_t Size;
    ino_t INode;
    char *Name;

    DirectoryEntry(mode_t mode, time_t atime, time_t ctime, time_t mtime, size64_t size, ino_t inode, const char *name);
    ~DirectoryEntry();
};

#endif // DIRECTORYENTRY_H
