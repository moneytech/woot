#ifndef VOLUMETYPE_H
#define VOLUMETYPE_H

#include <list.h>
#include <types.h>

class Drive;
class Mutex;
class Volume;

class VolumeType
{
    static List<VolumeType *> *volumeTypes;
    static Mutex *listLock;
public:
    char *Name;

    static void Initialize();
    static bool LockList();
    static bool Add(VolumeType *type);
    static VolumeType *GetByName(const char *name);
    static VolumeType *GetByIndex(uint idx);
    static void Remove(VolumeType *type);
    static int AutoDetect();
    static void UnLockList();
    static void Cleanup();

    VolumeType(const char *name);
    virtual int Detect(Drive *drive);
    virtual bool Compare(Volume *a, Volume *b);
    ~VolumeType();
};

#endif // VOLUMETYPE_H
