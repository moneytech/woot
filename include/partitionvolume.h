#ifndef PARTITIONVOLUME_H
#define PARTITIONVOLUME_H

#include <bufferedvolume.h>
#include <types.h>
#include <volumetype.h>

class PartitionVolumeType : public VolumeType
{
public:
    PartitionVolumeType();
    virtual int Detect(Drive *drive);
    virtual bool Compare(Volume *a, Volume *b);
};

class PartitionVolume : public BufferedVolume
{
    friend class PartitionVolumeType;

    size64_t firstSector;
    size64_t sectorCount;
public:
    PartitionVolume(class Drive *drive, VolumeType *type, size64_t firstSector, size64_t sectorCount);
    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count);
    ~PartitionVolume();
};

#endif // PARTITIONVOLUME_H
