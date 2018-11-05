#ifndef DEBUGSTREAM_H
#define DEBUGSTREAM_H

#include <stream.h>

class FrameBuffer;

class DebugStream : public Stream
{
    word port;
    FrameBuffer *fb;
    int fbX, fbY, fbW, fbH;
    bool lineBufferState;
    char lineBuffer[256];
    int lineBufferPos;
public:
    DebugStream(word port);
    void SetFrameBuffer(FrameBuffer *fb);
    void EnableLineBuffer();
    void DisableLineBuffer();
    virtual int64_t Read(void *buffer, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t n);
    virtual ~DebugStream();
};

#endif // DEBUGSTREAM_H
