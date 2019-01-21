#ifndef BUFFEREDVOLUME_H
#define BUFFEREDVOLUME_H

#include <sequencer.h>
#include <types.h>
#include <volume.h>

class BufferedVolume : public Volume
{
    struct BlockBuffer
    {
        byte *Buffer = nullptr;
        uint64_t FirstSector = 0;
        bool Used = false;
        bool Dirty = false;
        uint64_t Time = 0; // Used for LRU algorithm
    };

    static Sequencer<uint64_t> lru;

    size_t sectorsPerBuffer;
    size_t bufferCount;
    BlockBuffer *buffers;

    int getContainingBuffer(uint64_t sector);
    bool flushBuffer(int bufIdx);
    int allocateBuffer();
    bool loadBuffer(int buffer, int64_t sector);
    int64_t access(byte *buffer, int64_t n, int64_t position, bool write);
protected:
    BufferedVolume(class Drive *drive, VolumeType *type, size_t sectorsPerBuffer, size_t bufferCount);
    virtual ~BufferedVolume();
public:
    virtual int64_t Read(void *buffer, uint64_t position, int64_t n);
    virtual int64_t Write(const void *buffer, uint64_t position, int64_t n);
    bool Flush_nolock();
};

#endif // BUFFEREDVOLUME_H
