#ifndef DRIVE_H
#define DRIVE_H

#include <list.h>
#include <sequencer.h>
#include <types.h>

class Mutex;

class Drive
{
    static Sequencer<int> id;
    static List<Drive *> *drives;
    static Mutex *lock;
public:
    static void Initialize();
    static bool LockList();
    static Drive *GetByID(int id);
    static Drive *GetByIndex(uint idx);
    static int Add(Drive *drive);
    static bool Remove(Drive *drive);
    static void UnLockList();
    static void Cleanup();

    int ID;
    size_t SectorSize;
    size64_t SectorCount;
    size64_t Size;
    char *Model;
    char *Serial;

    Drive(size_t sectorSize, size64_t sectorCount, const char *model, const char *serial);
    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count);
    virtual bool HasMedia();
    virtual ~Drive();
};

#endif // DRIVE_H
