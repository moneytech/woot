#ifndef MEMORYSTREAM_H
#define MEMORYSTREAM_H

#include <types.h>
#include <stream.h>

class MemoryStream : public Stream
{
protected:
    byte *data = nullptr;
    int64_t size = 0;
    int64_t position = 0;
public:
    MemoryStream(void *base, int64_t size);
    void *GetData();
    virtual int64_t Read(void *buffer, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t n);
};

#endif // MEMORYSTREAM_H
