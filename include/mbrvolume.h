#ifndef MBRVOLUME_H
#define MBRVOLUME_H

#include <bufferedvolume.h>
#include <types.h>
#include <volumetype.h>

class MBRVolumeType : public VolumeType
{
public:
    MBRVolumeType();
    virtual int Detect(Drive *drive);
    virtual bool Compare(Volume *a, Volume *b);
};

class MBRVolume : public BufferedVolume
{
    friend class MBRVolumeType;

    size64_t firstSector;
    size64_t sectorCount;
public:
    MBRVolume(class Drive *drive, VolumeType *type, size64_t firstSector, size64_t sectorCount);
    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count);
    ~MBRVolume();
};

#endif // MBRVOLUME_H
