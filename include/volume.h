#ifndef VOLUME_H
#define VOLUME_H

#include <list.h>
#include <sequencer.h>
#include <types.h>

class Drive;
class Mutex;
class VolumeType;

class Volume
{
    static Sequencer<int> id;
    static List<Volume *> *volumes;
    static Mutex *listLock;
    Mutex *lock;
protected:
    Volume(class Drive *drive, VolumeType *type);
    virtual ~Volume();
public:
    int ID;
    Drive *Drive;
    VolumeType *Type;

    static void Initialize();
    static bool LockList();
    static Volume *GetByID(int id, bool lock);
    static Volume *GetByIndex(uint idx, bool lock);
    static int Add(Volume *vol);
    static bool Remove(Volume *vol);
    static void UnLockList();
    static void Cleanup();

    bool Lock();
    virtual int64_t Read(void *buffer, uint64_t position, int64_t n); // may be buferred
    virtual int64_t Write(const void *buffer, uint64_t position, int64_t n); // may be buferred
    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);  // unbuffered read
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count); // unbuffered write
    virtual bool Flush(); // synchronizes buffers with media
    void UnLock();
};

#endif // VOLUME_H
