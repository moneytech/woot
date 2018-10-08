#include <string.h>
#include <memorystream.h>

MemoryStream::MemoryStream(void *base, int64_t size) :
    data((byte *)base), size(size)
{
}

void *MemoryStream::GetData()
{
    return data;
}

int64_t MemoryStream::Read(void *buffer, int64_t n)
{
    if(position + n > size)
        n = size - position;
    if(n == 1) *(byte *)buffer = data[position];
    else memcpy(buffer, data + position, n);
    position += n;
    return n;
}

int64_t MemoryStream::Write(const void *buffer, int64_t n)
{
    if(position + n > size)
        n = size - position;
    if(n == 1) data[position] = *(byte *)buffer;
    else memcpy(data + position, buffer, n);
    position += n;
    return n;
}
