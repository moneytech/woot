#include <drive.h>
#include <errno.h>
#include <limits.h>
#include <partitionvolume.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef struct MBRTableEntry
{
    byte Flags;
    byte StartCHS[3];
    byte ID;
    byte EndCHS[3];
    uint32_t StartLBA;
    uint32_t LBACount;
} MBRTableEntry;
#pragma pack(pop)

PartitionVolume::PartitionVolume(class Drive *drive, VolumeType *type, size64_t firstSector, size64_t sectorCount) :
    BufferedVolume(drive, type, 16, 16),
    firstSector(firstSector),
    sectorCount(sectorCount)
{
}

int64_t PartitionVolume::ReadSectors(void *buffer, uint64_t start, int64_t count)
{
    if(!Drive) return -EINVAL;
    if(start >= sectorCount) return 0;

    if((start + count) > sectorCount)
        count = sectorCount - start;
    return Drive->ReadSectors(buffer, start + firstSector, count);
}

int64_t PartitionVolume::WriteSectors(const void *buffer, uint64_t start, int64_t count)
{
    if(!Drive) return -EINVAL;
    if(start >= sectorCount) return 0;

    if((start + count) > sectorCount)
        count = sectorCount - start;
    return Drive->WriteSectors(buffer, start + firstSector, count);
}

PartitionVolume::~PartitionVolume()
{
}

PartitionVolumeType::PartitionVolumeType() :
    VolumeType("partition")
{
}

int PartitionVolumeType::Detect(Drive *drive)
{
    //printf("[partitionvolume] Detect()\n");
    if(!drive) return -EINVAL;
    if(drive->SectorSize < 512) return 0;
    byte *firstSectorData = new byte[drive->SectorSize];
    if(drive->ReadSectors(firstSectorData, 0, 1) != 1)
    {
        delete[] firstSectorData;
        printf("[partitionvolume] Couldn't read first sector on drive %d\n", drive->ID);
        return -EIO;
    }
    int res = 0;
    if(firstSectorData[510] != 0x55 || firstSectorData[511] != 0xAA)
    {
        delete[] firstSectorData;
        return 0;
    }
    MBRTableEntry *mbrt = (MBRTableEntry *)(firstSectorData + 446);

    // check mbr integrity
    bool mbrOk = true;
    int activeCount = 0;
    int entryCount = 0;
    for(int i = 0; i < 4; ++i)
    {
        if(!mbrt[i].StartLBA)
            continue;
        if(mbrt[i].Flags & 0x80)
            ++activeCount;
        if((mbrt[i].StartLBA + mbrt[i].LBACount) > drive->SectorCount)
        {
            mbrOk = false;
            break;
        }
        // TODO: Add CHS and LBA equality check
        ++entryCount;
    }

    if(drive->SectorSize != 512 && mbrt[0].ID == 0xCD)
        mbrOk = false; // we have hybrid cd image

    if(mbrt[0].ID == 0xEE)
        mbrOk = false; // we have GPT partitioning

    if(!entryCount || activeCount > 1)
        mbrOk = false; // no partitions or more than one active partition

    if(!mbrOk)
    {   // unkonwn partition scheme or not partitioned drive
        delete[] firstSectorData;
        PartitionVolume *vol = new PartitionVolume(drive, this, 0, SIZE_MAX);
        int id = Volume::Add(vol);
        printf("[partitionvolume] Found non partitioned volume (id: %d) on drive %d\n", id, drive->ID);
        return 1;
    }

    for(int i = 0; i < 4; ++i)
    {
        if(!mbrt[i].StartLBA)
            continue;

        PartitionVolume *vol = new PartitionVolume(drive, this, mbrt[i].StartLBA, mbrt[i].LBACount);
        int id = Volume::Add(vol);
        printf("[partitionvolume] Found MBR partition of type %#.2x (id: %d) on drive %d\n", mbrt[i].ID, id, drive->ID);
        ++res;
    }


    delete[] firstSectorData;
    return res;
}

bool PartitionVolumeType::Compare(Volume *a, Volume *b)
{
    if(a->Type != this || a->Type != b->Type || a->Drive != b->Drive)
        return false;
    PartitionVolume *A = (PartitionVolume *)a, *B = (PartitionVolume *)b;
    return A->firstSector == B->firstSector && A->sectorCount == B->sectorCount;
}
