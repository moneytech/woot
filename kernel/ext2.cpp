#include <ext2.h>
#include <stdio.h>
#include <volume.h>

EXT2FileSystemType::EXT2FileSystemType() :
    FileSystemType("ext2")
{

}

FileSystem *EXT2FileSystemType::Detect(Volume *vol)
{
    if(!vol) return nullptr;
    EXT2::SuperBlock *sblock = new EXT2::SuperBlock;
    vol->Lock();
    if(vol->Read(sblock, 1024, sizeof(EXT2::SuperBlock)) != sizeof(EXT2::SuperBlock))
    {
        delete sblock;
        vol->UnLock();
        return nullptr;
    }

    if(sblock->s_magic != EXT2_SUPER_MAGIC)
    {
        delete sblock;
        vol->UnLock();
        return nullptr;
    }


    if((sblock->s_feature_incompat & EXT2_FEATURE_INCOMPAT_COMPRESSION) ||
            //(sblock->s_feature_incompat & EXT2_FEATURE_INCOMPAT_FILETYPE) ||
            (sblock->s_feature_incompat & EXT3_FEATURE_INCOMPAT_RECOVER) ||
            (sblock->s_feature_incompat & EXT3_FEATURE_INCOMPAT_JOURNAL_DEV) ||
            (sblock->s_feature_incompat & EXT2_FEATURE_INCOMPAT_META_BG) ||
            (sblock->s_feature_incompat & EXTx_FEATURE_INCOMPAT_FUTURE))
    {
        printf("[ext2] Detected incompatible features (%#.4x) on volume %d\n", sblock->s_feature_incompat, vol->ID);
        delete sblock;
        vol->UnLock();
        return nullptr;
    }

    bool readOnly = false;
    if(//(sblock->s_feature_incompat & EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER) ||
            //(sblock->s_feature_incompat & EXT2_FEATURE_RO_COMPAT_LARGE_FILE) ||
            (sblock->s_feature_incompat & EXT2_FEATURE_RO_COMPAT_BTREE_DIR) ||
            (sblock->s_feature_incompat & EXTx_FEATURE_RO_COMPAT_FUTURE))
    {
        printf("[ext2] Detected ro compatible features (%#.4x) on volume %d\n", sblock->s_feature_incompat, vol->ID);
        printf("       mounting as read only\n");
        readOnly = true;
    }

    printf("[ext2] Found valid filesystem on volume %d\n", vol->ID);

    EXT2 *fs = new EXT2(vol, this, sblock, readOnly);
    FileSystem::Add(fs);

    vol->UnLock();
    delete sblock;
    return nullptr;
}

EXT2::EXT2(class Volume *vol, FileSystemType *type, EXT2::SuperBlock *sblock, bool ro) :
    FileSystem(vol, type)
{

}

EXT2::~EXT2()
{

}
