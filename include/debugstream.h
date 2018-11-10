#ifndef DEBUGSTREAM_H
#define DEBUGSTREAM_H

#include <stream.h>
#include <kwm.h>

class PixMap;

class DebugStream : public Stream
{
    word port;
    WindowManager::Window *window;
    int fbX, fbY, winW, winH;
    bool lineBufferState;
    char lineBuffer[256];
    int lineBufferPos;
public:
    DebugStream(word port);
    void SetWindow(WindowManager::Window *window);
    void EnableLineBuffer();
    void DisableLineBuffer();
    virtual int64_t Read(void *buffer, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t n);
    virtual ~DebugStream();
};

#endif // DEBUGSTREAM_H
