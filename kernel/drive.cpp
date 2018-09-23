#include <cpu.h>
#include <drive.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

Sequencer<int> Drive::id(0);
List<Drive *> *Drive::drives;

void Drive::Initialize()
{
    drives = new List<Drive *>();
}

Drive *Drive::GetByID(int id)
{
    bool ints = cpuDisableInterrupts();
    Drive *res = nullptr;
    for(auto it : *drives)
    {
        if(it->ID == id)
        {
            res = it;
            break;
        }
    }
    cpuRestoreInterrupts(ints);
    return res;
}

Drive *Drive::GetByIndex(uint idx)
{
    bool ints = cpuDisableInterrupts();
    Drive *drive = drives->Get(idx);
    cpuRestoreInterrupts(ints);
    return drive;
}

int Drive::Add(Drive *drive)
{
    bool ints = cpuDisableInterrupts();
    int newId = id.GetNext();
    drives->Append(drive);
    drive->ID = newId;
    cpuRestoreInterrupts(ints);
    return newId;
}

bool Drive::Remove(Drive *drive)
{
    bool ints = cpuDisableInterrupts();
    bool res = drives->Remove(drive, nullptr, false) != 0;
    drive->ID = -1;
    cpuRestoreInterrupts(ints);
    return res;
}

void Drive::Cleaup()
{
    for(auto n : *drives)
    {
        if(n) delete n;
    }
    delete drives;
}

Drive::Drive(size_t sectorSize, size64_t sectorCount, const char *model, const char *serial) :
    ID(-1),
    SectorSize(sectorSize),
    SectorCount(sectorCount),
    Size(SectorSize * SectorCount),
    Model(model ? strdup(model) : nullptr),
    Serial(serial ? strdup(serial) : nullptr)
{
}

int64_t Drive::ReadSectors(void *buffer, uint64_t start, int64_t count)
{
    return -ENOSYS;
}

int64_t Drive::WriteSectors(const void *buffer, uint64_t start, int64_t count)
{
    return -ENOSYS;
}

bool Drive::HasMedia()
{
    return false;
}

Drive::~Drive()
{
    if(Model) free(Model);
    if(Serial) free(Serial);
}
