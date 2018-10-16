#ifndef FILESTREAM_H
#define FILESTREAM_H

#include <file.h>
#include <stream.h>
#include <types.h>

class File;

class FileStream : public Stream
{
    File *file;
public:
    FileStream(File *file);
    virtual int64_t Read(void *buffer, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t n);
};

#endif // FILESTREAM_H
