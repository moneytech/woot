#ifndef FILE_H
#define FILE_H

#include <types.h>

#define O_RDONLY    00
#define O_WRONLY    01
#define O_RDWR      02
#define O_ACCMODE   03
#define O_CREAT     0100
#define O_EXCL      0200
#define O_NOCTTY    0400
#define O_TRUNC     01000
#define O_APPEND    02000
#define O_NONBLOCK  04000
#define O_DSYNC     010000
#define O_SYNC      04010000
#define O_RSYNC     04010000
#define O_DIRECTORY 0200000
#define O_NOFOLLOW  0400000
#define O_CLOEXEC   02000000
#define O_ASYNC     020000
#define O_DIRECT    040000
#define O_LARGEFILE 0100000
#define O_NOATIME   01000000
#define O_PATH      010000000
#define O_TMPFILE   020200000
#define O_NDELAY    O_NONBLOCK

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

class DEntry;
class DirectoryEntry;
class Mutex;

class File
{
    File(::DEntry *dentry, int flags);
public:
    ::DEntry *DEntry;
    int Flags;
    int64_t Position;
    Mutex *Lock;

    static File *Open(::DEntry *parent, const char *name, int flags);
    static File *Open(const char *name, int flags);

    int64_t GetSize();
    bool SetAccessTime(time_t time);
    bool SetModifyTime(time_t time);
    bool Create(const char *name, mode_t mode);
    int64_t Seek(int64_t offs, int loc);
    int64_t Read(void *buffer, int64_t n);
    int64_t Write(const void *buffer, int64_t n);
    int64_t Rewind();
    DirectoryEntry *ReadDir();
    ~File(); // used as close
};

#endif // FILE_H
