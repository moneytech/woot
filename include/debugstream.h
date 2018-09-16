#ifndef DEBUGSTREAM_H
#define DEBUGSTREAM_H

#include <stream.h>

class DebugStream : public Stream
{
    word port;
public:
    DebugStream(word port);
    virtual int64_t Read(void *buffer, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t n);
    virtual ~DebugStream();
};

#endif // DEBUGSTREAM_H
