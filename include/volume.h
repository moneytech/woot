#ifndef VOLUME_H
#define VOLUME_H

#include <list.h>
#include <sequencer.h>
#include <types.h>

class Drive;
class FileSystem;
class Mutex;
class UUID;
class VolumeType;

class Volume
{
    static Sequencer<int> id;
    static List<Volume *> volumes;
    static Mutex lock;
protected:
    Volume(class Drive *drive, VolumeType *type);
    virtual ~Volume();
public:
    int ID;
    ::Drive *Drive;
    VolumeType *Type;
    FileSystem *FS = nullptr;

    static void Initialize();
    static bool Lock();
    static Volume *GetByID_nolock(int id);
    static Volume *GetByID(int id);
    static Volume *GetByIndex_nolock(uint idx);
    static Volume *GetByIndex(uint idx);
    static Volume *GetByLabel_nolock(const char *label);
    static Volume *GetByLabel(const char *label);
    static Volume *GetByUUID_nolock(UUID uuid);
    static Volume *GetByUUID(UUID uuid);
    static int Add(Volume *vol);
    static bool Remove(Volume *vol);
    static void UnLock();
    static void FlushAll();
    static void Cleanup();

    virtual int64_t Read(void *buffer, uint64_t position, int64_t n); // may be buferred
    virtual int64_t Write(const void *buffer, uint64_t position, int64_t n); // may be buferred
    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);  // unbuffered read
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count); // unbuffered write
    virtual bool Flush(); // synchronizes buffers with media
};

#endif // VOLUME_H
