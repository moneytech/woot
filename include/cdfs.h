#ifndef CDFS_H
#define CDFS_H

#include <filesystem.h>
#include <filesystemtype.h>
#include <inode.h>
#include <types.h>

class Volume;

#define CDFS_VDT_BOOT_RECORD                     0
#define CDFS_VDT_PRIMARY_VOLUME_DESCRIPTOR       1
#define CDFS_VDT_SUPPLEMENTARY_VOLUME_DESCRIPTOR 2
#define CDFS_VDT_VOLUME_PARTITION_DESCRIPTOR     3
#define CDFS_VDT_TERMINATOR                      255

#define CDFS_FLAG_HIDDEN     (1 << 0)
#define CDFS_FLAG_DIRECTORY  (1 << 1)
#define CDFS_FLAG_ASSOCIATED (1 << 2)
#define CDFS_FLAG_EXT_ATTR   (1 << 3)
#define CDFS_FLAG_PERM_ATTR  (1 << 4)
#define CDFS_FLAG_NOT_LAST   (1 << 7)

class CDFSFileSystemType : public FileSystemType
{
public:
    CDFSFileSystemType();
    virtual FileSystem *Detect(Volume *vol);
};

class CDFS : public FileSystem
{
    friend class CDFSFileSystemType;
#pragma pack(push, 1)
    typedef uint8_t int8;
    typedef int8_t sint8;

    typedef uint16_t int16_LSB;
    typedef uint16_t int16_MSB;
    struct int16_LSB_MSB
    {
        union
        {
            int16_LSB CPU;
            int16_LSB LSB;
        };
        int16_MSB MSB;
    };

    typedef int16_t sint16_LSB;
    typedef int16_t sint16_MSB;
    struct sint16_LSB_MSB
    {
        union
        {
            sint16_LSB CPU;
            sint16_LSB LSB;
        };
        sint16_MSB MSB;
    };

    typedef uint32_t int32_LSB;
    typedef uint32_t int32_MSB;
    struct int32_LSB_MSB
    {
        union
        {
            int32_LSB CPU;
            int32_LSB LSB;
        };
        int32_MSB MSB;
    };

    typedef int32_t sint32_LSB;
    typedef int32_t sint32_MSB;
    struct sint32_LSB_MSB
    {
        union
        {
            sint32_LSB CPU;
            sint32_LSB LSB;
        };
        sint32_MSB MSB;
    };

    struct DateTime
    {
        char Year[4];
        char Month[2];
        char Day[2];
        char Hour[2];
        char Minute[2];
        char Second[2];
        char Hundreds[2];
        int8 TimeZone;
    };

    struct DirectoryDateTime
    {
        int8 Year;
        int8 Month;
        int8 Day;
        int8 Hour;
        int8 Minute;
        int8 Second;
        sint8 TimeZone;
    };

    struct DescriptorHeader
    {
        int8 Type;
        char Identifier[5];
        int8 Version;
    };

    struct BootRecord
    {
        DescriptorHeader Header;
        byte BootSystemIdentifier[32];
        byte BootIdentifier[32];
        byte BootSystemUse[0];
    };

    struct DirectoryEntry
    {
        int8 DirectoryRecordLength;
        int8 ExtendedAttributeRecordLength;
        int32_LSB_MSB LocationOfExtent;
        int32_LSB_MSB DataLength;
        DirectoryDateTime RecordingDateTime;
        int8 Flags;
        int8 InterleavedFileUnitSize;
        int8 InterleaveGapSize;
        int16_LSB_MSB VolumeSequenceNumber;
        int8 FileIdentifierLength;
        char FileIdentifier[0];
    };

    struct PrimaryVolumeDescriptor
    {
        DescriptorHeader Header;
        int8 Unused1;
        char SystemIdentifier[32];
        char VolumeIdentifier[32];
        byte Unused2[8];
        int32_LSB_MSB VolumeSpaceSize;
        byte Unused3[32];
        int16_LSB_MSB VolumeSetSize;
        int16_LSB_MSB VolumeSequenceNumber;
        int16_LSB_MSB LogicalBlockSize;
        int32_LSB_MSB PathTableSize;
        struct
        {
            union
            {
                struct
                {
                    int32_LSB PathTableLocation_CPU;
                    int32_LSB OptionalPathTableLocation_CPU;
                };
                struct
                {
                    int32_LSB PathTableLocation_LSB;
                    int32_LSB OptionalPathTableLocation_LSB;
                };
            };
            int32_MSB PathTableLocation_MSB;
            int32_MSB OptionalPathTableLocation_MSB;
        };
        byte RootDirectoryEntryData[34];
        char VolumeSetIdentifier[128];
        char PublisherIdentifier[128];
        char DataPreparerIdentifier[128];
        char ApplicationIdentifier[128];
        char CopyrightFileIdentifier[38];
        char AbstractFileIdentifier[36];
        char BibliographicFileIdentifier[37];
        DateTime CreationDateTime;
        DateTime ModificationDateTime;
        DateTime ExpirationDateTime;
        DateTime EffectiveDateTime;
        int8 FileStructureVersion;
        byte Unused4;
        byte ApplicationUsed[512];
        byte Reserved[653];
    };

    struct PathTableEntry
    {
        int8 DirectoryIdentifierLength;
        int8 ExtendedAttributeRecordLength;
        union
        {
            union
            {
                int32_LSB LocationOfExtent_CPU;
                int32_LSB LocationOfExtent_LSB;
            };
            int32_MSB LocationOfExtent_MSB;
        };
        union
        {
            union
            {
                int32_LSB ParentDirectoryNumber_CPU;
                int32_LSB ParentDirectoryNumber_LSB;
            };
            int32_MSB ParentDirectoryNumber_MSB;
        };
        char DirectoryIdentifier[0];
    };
#pragma pack(pop)

    class FSINode : public INode
    {
        friend class CDFS;
        DirectoryEntry DirEntry;
        mode_t SUSPMode;

        FSINode(ino_t number, FileSystem *fs);
        virtual size64_t GetSize();
        virtual mode_t GetMode();
        virtual time_t GetCreateTime();
        virtual time_t GetModifyTime();
        virtual time_t GetAccessTime();
        virtual int GetLinkCount();
        virtual uid_t GetUID();
        virtual gid_t GetGID();
        virtual int64_t Read(void *buffer, int64_t position, int64_t n);
        virtual ::DirectoryEntry *ReadDir(int64_t position, int64_t *newPosition);
    };

    size_t sectSize;
    PrimaryVolumeDescriptor pvd;
    DirectoryEntry *rde;
    bool bootable;
    bool initialized;
    char *label;

    CDFS(class Volume *vol, FileSystemType *type);
    ~CDFS();
public:
    virtual bool GetLabel(char *buffer, size_t num);
    virtual class INode *ReadINode(ino_t number);
};

#endif // CDFS_H
