#include <cdfs.h>
#include <dentry.h>
#include <directoryentry.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <volume.h>

#define SUSPID_NM 0x4D4E
#define SUSPID_PX 0x5850

CDFSFileSystemType::CDFSFileSystemType() :
    FileSystemType("cdfs")
{
}

FileSystem *CDFSFileSystemType::Detect(Volume *vol)
{
    size_t sectSize = vol->GetSectorSize();
    if(!sectSize) return nullptr;
    CDFS::DescriptorHeader hdr;
    int64_t br = vol->Read(&hdr, sectSize * 0x10, sizeof(hdr));
    if(br != sizeof(hdr))
        return nullptr;
    if(memcmp("CD001", hdr.Identifier, 5))
        return nullptr;
    printf("[cdfs] Found ISO9660 filesystem on volume\n", vol->ID);
    return new CDFS(vol, this);
}

CDFS::CDFS(class Volume *vol, FileSystemType *type) :
    FileSystem(vol, type),
    sectSize(vol->GetSectorSize()),
    bootable(false),
    initialized(false)
{
    DescriptorHeader hdr;
    ino_t rootIno = 0;
    for(int i = 0; i < 64; ++i)
    {
        int64_t br = vol->Read(&hdr, sectSize * (0x10 + i), sizeof(hdr));
        if(br != sizeof(hdr))
            return;
        if(hdr.Type == CDFS_VDT_TERMINATOR)
            break;
        else if(hdr.Type == CDFS_VDT_BOOT_RECORD)
        {
            bootable = true;
            continue;
        }
        else if(hdr.Type == CDFS_VDT_PRIMARY_VOLUME_DESCRIPTOR)
        {
            br = vol->Read(&pvd, sectSize * (0x10 + i), sizeof(pvd));
            if(br != sizeof(pvd))
                return;
            rde = (DirectoryEntry *)pvd.RootDirectoryEntryData;
            rootIno = sectSize * (0x10 + i) + offsetof(PrimaryVolumeDescriptor, RootDirectoryEntryData);
            initialized = true;
            continue;
        }
        //else printf("  hdr.Type: %d\n", hdr.Type);
    }

    INode *rootINode = GetINode(rootIno);
    if(!rootINode)
    {
        initialized = false;
        return;
    }
    DEntry *root = new DEntry("/", nullptr, rootINode);
    SetRoot(root);

    printf("       bootable: %s\n", bootable ? "yes" : "no");
    printf("       initialized: %s\n", initialized ? "yes" : "no");

    label = (char *)calloc(1, 36);
    if(label)
    {
        memcpy(label, pvd.VolumeIdentifier, 32);
        for(int i = 31; i > 0; --i)
        {
            if(label[i] != ' ')
                break;
            label[i] = 0;
        }
        printf("       label: %s\n", label);
    }
}

CDFS::~CDFS()
{
    if(label) free(label);
}

bool CDFS::GetLabel(char *buffer, size_t num)
{
    if(!label) return false;
    memset(buffer, 0, num);
    strncpy(buffer, label, min(32, num));
    return true;
}

class INode *CDFS::ReadINode(ino_t number)
{
    //printf("[cdfs] readinode: %#lx\n", number);

    DirectoryEntry de;
    int64_t br = Volume->Read(&de, number, sizeof(de));
    if(br != sizeof(de))
        return nullptr;

    // do some sanity check
    if(de.DirectoryRecordLength < 32 || de.RecordingDateTime.Month < 1 ||
            de.RecordingDateTime.Month > 12 || de.RecordingDateTime.Day < 1 ||
            de.RecordingDateTime.Day > 31 || de.RecordingDateTime.Hour > 23 ||
            de.RecordingDateTime.Minute > 59 || de.RecordingDateTime.Second > 59 ||
            de.RecordingDateTime.TimeZone < -48 || de.RecordingDateTime.TimeZone > 52)
        return nullptr;

    FSINode *inode = new FSINode(number, this);
    memcpy(&inode->DirEntry, &de, sizeof(de));
    return inode;
}

CDFS::FSINode::FSINode(ino_t number, FileSystem *fs) :
    INode(number, fs)
{
}

size64_t CDFS::FSINode::GetSize()
{
    return DirEntry.DataLength.CPU;
}

mode_t CDFS::FSINode::GetMode()
{
    return SUSPMode ? SUSPMode : (0755 | (DirEntry.Flags & CDFS_FLAG_DIRECTORY ? S_IFDIR : 0));
}

time_t CDFS::FSINode::GetCreateTime()
{
    Time::DateTime dt;
    dt.Year = 1900 + DirEntry.RecordingDateTime.Year;
    dt.Month = DirEntry.RecordingDateTime.Month;
    dt.Day = DirEntry.RecordingDateTime.Day;
    dt.Hour = DirEntry.RecordingDateTime.Hour;
    dt.Minute = DirEntry.RecordingDateTime.Minute;
    dt.Second = DirEntry.RecordingDateTime.Second;
    return Time::DateTimeToUnix(&dt);
}

time_t CDFS::FSINode::GetModifyTime()
{
    return GetCreateTime();
}

time_t CDFS::FSINode::GetAccessTime()
{
    return GetCreateTime();
}

int CDFS::FSINode::GetLinkCount()
{
    return 1;
}

uid_t CDFS::FSINode::GetUID()
{
    return 0;
}

gid_t CDFS::FSINode::GetGID()
{
    return 0;
}

int64_t CDFS::FSINode::Read(void *buffer, int64_t position, int64_t n)
{
    size64_t size = GetSize();
    if((position + n) > size)
        n = size - position;
    if(!n) return 0;
    CDFS *fs = (CDFS *)FS;
    return fs->Volume->Read(buffer, DirEntry.LocationOfExtent.CPU * fs->sectSize + position, n);
}

::DirectoryEntry *CDFS::FSINode::ReadDir(int64_t position, int64_t *newPosition)
{
    if(!(GetMode() & S_IFDIR))
    { // we can't do ReadDir for non-directory
        return nullptr;
    }

    DirectoryEntry de;
    char nameBuf[224];
    char suspName[224];
    mode_t suspMode = 0;
    size_t size = GetSize();
    ::DirectoryEntry *res = nullptr;
    CDFS *fs = (CDFS *)FS;

    while(position < size)
    {
        int8 del = 0;
        if(Read(&del, position, 1) != 1)
            break;
        if(!del) position = align(position, fs->sectSize);
        if(position >= size) break;

        if(Read(&de, position, sizeof(de)) != sizeof(de))
            break;
        if(!de.DirectoryRecordLength)
            break;
        memset(nameBuf, 0, sizeof(nameBuf));
        int btr = de.DirectoryRecordLength - sizeof(de);
        if(Read(nameBuf, position + sizeof(de), btr) != btr)
            break;
        size_t padding = 1 - (de.FileIdentifierLength & 1);
        ssize_t suspSize = btr - (de.FileIdentifierLength + padding);
        bool hasRRName = false;
        if(suspSize > 0)
        {
            int suspOffs = 0;
            byte *suspData = (byte *)(nameBuf + de.FileIdentifierLength + padding);
            while(suspOffs < suspSize)
            {
                uint16_t type = *(uint16_t *)suspData;
                if(type == SUSPID_NM)
                {
                    hasRRName = true;
                    int suspNameLen = suspData[2] - 5;
                    memcpy(suspName, suspData + 5, suspNameLen);
                    suspName[suspNameLen] = 0;
                }
                else if(type == SUSPID_PX)
                    suspMode = *(mode_t *)(suspData + 4);
                int incr = max(suspData[2], 4);
                suspData += incr;
                suspOffs += incr;
            }
        }

        if(!hasRRName)
        {
            // build normal filename
            if(!nameBuf[0])
                strcpy(nameBuf, ".");
            else if(nameBuf[0] == 1)
                strcpy(nameBuf, "..");
            else
            {
                char *semi = strrchr(nameBuf, ';');
                if(semi) *semi = 0;
                char *dot = strrchr(nameBuf, '.');
                if(dot && !dot[1])
                    *dot = 0;
            }
        }
        else strncpy(nameBuf, suspName, sizeof(nameBuf));

        // fabricobble an inode
        FSINode inode(DirEntry.LocationOfExtent.CPU * fs->sectSize + position, FS);
        memcpy(&inode.DirEntry, &de, sizeof(de));
        if(suspMode) inode.SUSPMode = suspMode;
        res = new ::DirectoryEntry(
                    inode.GetMode(),
                    inode.GetAccessTime(),
                    inode.GetCreateTime(),
                    inode.GetModifyTime(),
                    inode.GetSize(),
                    inode.Number,
                    nameBuf);
        position += de.DirectoryRecordLength;
        break;
    }

    if(newPosition)
        *newPosition = position;
    return res;
}
