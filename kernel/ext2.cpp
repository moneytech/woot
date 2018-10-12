#include <dentry.h>
#include <directoryentry.h>
#include <errno.h>
#include <ext2.h>
#include <file.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <volume.h>
#include <thread.h>

// these 2 functions were taken from linux ext2 driver (balloc.c)
static inline int test_root(int a, int b)
{
    int num = b;

    while (a > num)
        num *= b;
    return num == a;
}

static int ext2_group_sparse(int group)
{
    if (group <= 1)
        return 1;
    return (test_root(group, 3) || test_root(group, 5) ||
        test_root(group, 7));
}

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
    vol->UnLock();
    delete sblock;
    return fs;
}

uint8_t EXT2::modeToFileType(uint32_t mode)
{
    switch(mode & EXT2_S_IFTYPE)
    {
    default:
        return EXT2_FT_UNKNOWN;
    case EXT2_S_IFIFO:
        return EXT2_FT_FIFO;
    case EXT2_S_IFCHR:
        return EXT2_FT_CHRDEV;
    case EXT2_S_IFDIR:
        return EXT2_FT_DIR;
    case EXT2_S_IFBLK:
        return EXT2_FT_BLKDEV;
    case EXT2_S_IFREG:
        return EXT2_FT_REG_FILE;
    case EXT2_S_IFLNK:
        return EXT2_FT_SYMLINK;
    case EXT2_S_IFSOCK:
        return EXT2_FT_SOCK;
    }
}

bool EXT2::isValidFileName(const char *name)
{
    int nameLen = strlen(name);
    if(nameLen > 255)
        return false; // name too long
    if(!strcmp(name, ".") || !strcmp(name, ".."))
        return false; // . and .. are reserved
    if(strchr(name, '/'))
        return false; // / is reserved as separator
    return true;
}

EXT2::EXT2(class Volume *vol, FileSystemType *type, EXT2::SuperBlock *sblock, bool ro) :
    FileSystem(vol, type),
    superBlock(new SuperBlock),
    readOnly((memcpy(superBlock, sblock, sizeof(SuperBlock)), ro)),
    blockSize(1024 << superBlock->s_log_block_size),
    fragSize(superBlock->s_log_frag_size < 0 ? (1024 >> -superBlock->s_log_frag_size) : (1024 << superBlock->s_log_frag_size)),
    totalSize((size64_t)blockSize * superBlock->s_blocks_count),
    blockGroupCount((superBlock->s_blocks_count - 1) / superBlock->s_blocks_per_group + 1),
    BGDT(new BlockGroupDescriptor[blockGroupCount]),
    BGDTSize(blockGroupCount * sizeof(BlockGroupDescriptor)),
    BGDTOffset((superBlock->s_first_data_block + 1) * blockSize),
    initialized(false), superDirty(false),
    blockOfZeros(new byte[blockSize])
{
    if(Volume->Read(BGDT, BGDTOffset, BGDTSize) != BGDTSize)
    {
        printf("[ext2] Couldn't read BGDT on volume %d\n", Volume->ID);
        return;
    }
    initialized = true;
    ::INode *rootINode = GetINode(EXT2_ROOT_INO);
    initialized = false;
    if(!rootINode)
    {
        printf("[ext2] Couldn't get root directory inode on volume %d\n", Volume->ID);
        return;
    }

    DEntry *root = new DEntry("/", nullptr);
    root->INode = rootINode;
    SetRoot(root);
    superBlock->s_mtime = time(nullptr);
    ++superBlock->s_mnt_count;
    superDirty = true;

    initialized = true;
}

uint64_t EXT2::getINodeOffset(ino_t n)
{
    uint32_t bg = (n - 1) / superBlock->s_inodes_per_group;
    uint64_t idx = (n - 1) % superBlock->s_inodes_per_group;
    return (uint64_t)BGDT[bg].bg_inode_table * blockSize + superBlock->s_inode_size * idx;
}

uint64_t EXT2::blockGroupOffset(uint bg)
{
    return (superBlock->s_first_data_block + superBlock->s_blocks_per_group * bg) * blockSize;
}

bool EXT2::hasSuperBlock(uint bg)
{
    if((superBlock->s_feature_ro_compat & EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER) && !ext2_group_sparse(bg))
        return false;
    return true;
}

uint64_t EXT2::bgdtOffset(uint bg)
{
    return blockGroupOffset(bg) + blockSize;
}

bool EXT2::updateBGDT(uint bg, off_t startOffs, size_t n)
{
    byte *bgdData = startOffs + (byte *)(BGDT + bg);
    for(uint i = 0; i < blockGroupCount; ++i)
    {
        if(!hasSuperBlock(i))
            continue;
        uint64_t offs = bgdtOffset(i) + sizeof(BlockGroupDescriptor) * bg + startOffs;
        if(Volume->Write(bgdData, offs, n) != n)
        {
            printf("Ext2 fatal error: couldn't update BGDT (group %d) on volume %d.\n"
                   "Filesystem may be inconsistent!\n"
                   "Use fsck.\n", i, Volume->ID);
            return false;
        }
    }
    return true;
}

uint EXT2::getINodeBlockGroup(uint32_t ino)
{
    return ino / superBlock->s_inodes_per_group;
}

uint32_t EXT2::allocINode(uint *group)
{
    for(uint i = 0; i < blockGroupCount; ++i)
    {
        BlockGroupDescriptor *bgd = BGDT + i;
        if(!bgd->bg_free_inodes_count)
            continue;
        for(int j = 0; j < (superBlock->s_inodes_per_group / 8); ++j)
        {
            uint64_t byteOffs = blockSize * bgd->bg_inode_bitmap + j;
            byte b;
            if(Volume->Read(&b, byteOffs, 1) != 1)
                return 0;
            if(b == 0xFF) continue; // no free inode in this byte
            // find lowest clear bit
            int bit = 0;
            for(bit = 0; bit < 8; ++bit)
            {
                if(!(b & (1 << bit)))
                    break;
            }
            // mark inode as used
            b |= (1 << bit);
            // update inode bitmap
            if(Volume->Write(&b, byteOffs, 1) != 1)
                return 0;
            // update block group descriptor
            --bgd->bg_free_inodes_count;
            if(!updateBGDT(i, offsetof(BlockGroupDescriptor, bg_free_inodes_count), 2))
                return 0;
            // update superblock
            --superBlock->s_free_inodes_count;
            superDirty = true;
            if(group) *group = i;
            return 1 + (i * superBlock->s_inodes_per_group) + (j * 8 + bit);
        }
    }
    return 0;
}

bool EXT2::freeINode(uint32_t inode)
{
    if(!inode) return false;
    --inode; // inode indices are 1 based

    int iGroup = inode / superBlock->s_blocks_per_group;
    int iOffs = (inode % superBlock->s_blocks_per_group) / 8;
    int iBit = (inode % superBlock->s_blocks_per_group) % 8;

    BlockGroupDescriptor *bgd = BGDT + iGroup;

    uint64_t byteOffs = blockSize * bgd->bg_inode_bitmap + iOffs;
    byte b;
    if(Volume->Read(&b, byteOffs, 1) != 1)
        return false;
    if(!(b & (1 << iBit)))
        return false; // inode is already free
    b &= ~(1 << iBit); // mark inode as free
    // update inode bitmap
    if(Volume->Write(&b, byteOffs, 1) != 1)
        return false;
    // update block group descriptor
    ++bgd->bg_free_inodes_count;
    if(!updateBGDT(iGroup, offsetof(BlockGroupDescriptor, bg_free_inodes_count), 2))
        return false;
    // update superblock
    ++superBlock->s_free_inodes_count;
    superDirty = true;
    return true;
}

uint32_t EXT2::allocBlockInGroup(uint g)
{
    BlockGroupDescriptor *bgd = BGDT + g;
    if(!bgd->bg_free_blocks_count)
        return 0;
    for(int j = 0; j < (superBlock->s_blocks_per_group / 8); ++j)
    {
        uint64_t byteOffs = blockSize * bgd->bg_block_bitmap + j;
        byte b;
        if(Volume->Read(&b, byteOffs, 1) != 1)
            return 0;
        if(b == 0xFF) continue; // no free block in this byte
        // find lowest clear bit
        int bit = 0;
        for(bit = 0; bit < 8; ++bit)
        {
            if(!(b & (1 << bit)))
                break;
        }
        // mark block as used
        b |= (1 << bit);
        // update block bitmap
        if(Volume->Write(&b, byteOffs, 1) != 1)
            return 0;
        // update block group descriptor
        --bgd->bg_free_blocks_count;
        if(!updateBGDT(g, offsetof(BlockGroupDescriptor, bg_free_blocks_count), 2))
            return 0;
        // update superblock
        --superBlock->s_free_blocks_count;
        superDirty = true;
        return 1 + (g * superBlock->s_blocks_per_group) + (j * 8 + bit);
    }
    return 0;
}

uint32_t EXT2::allocBlock(uint preferredGroup, uint *group)
{
    if(preferredGroup != ~0)
    {
        uint32_t blk = allocBlockInGroup(preferredGroup);
        if(group) *group = preferredGroup;
        if(blk) return blk;
    }
    for(int i = 0; i < blockGroupCount; ++i)
    {
        if(i == preferredGroup)
            continue; // we have already checked in that group
        uint32_t blk = allocBlockInGroup(i);
        if(group) *group = i;
        if(blk) return blk;
    }
    return 0;
}

bool EXT2::freeBlock(uint32_t block)
{
    if(!block) return false;
    --block;

    int bGroup = block / superBlock->s_blocks_per_group;
    int bOffs = (block % superBlock->s_blocks_per_group) / 8;
    int bBit = (block % superBlock->s_blocks_per_group) % 8;

    BlockGroupDescriptor *bgd = BGDT + bGroup;

    uint64_t byteOffs = blockSize * bgd->bg_block_bitmap + bOffs;
    byte b;
    if(Volume->Read(&b, byteOffs, 1) != 1)
        return false;
    if(!(b & (1 << bBit)))
        return false; // block is already free
    b &= ~(1 << bBit); // mark block as free
    // update block bitmap
    if(Volume->Write(&b, byteOffs, 1) != 1)
        return false;
    // update block group descriptor
    ++bgd->bg_free_blocks_count;
    if(updateBGDT(bGroup, offsetof(BlockGroupDescriptor, bg_free_blocks_count), 2))
        return false;
    // update superblock
    ++superBlock->s_free_blocks_count;
    superDirty = true;
    return true;
}

int64_t EXT2::read(FSINode *inode, void *buffer, uint64_t position, int64_t n)
{
    size64_t size = inode->GetSize();
    if((position + n) > size)
        n = size - position;
    if(!n) return 0;

    time_t curTime = time(nullptr);

    int64_t bytesLeft = n;
    byte *buf = (byte *)buffer;
    while(bytesLeft > 0)
    {
        int64_t blockIdx = getINodeBlock(inode, position / blockSize);
        if(!blockIdx)
            break;

        int64_t inBlockOffset = position % blockSize;
        int64_t blockOffset = blockIdx * blockSize;
        int64_t bytesToRead = min(bytesLeft, blockSize - inBlockOffset);
        int64_t bytesRead = Volume->Read(buf, blockOffset + inBlockOffset, bytesToRead);
        if(bytesRead < 0)
            return bytesRead;
        inode->SetAccessTime(curTime);
        position += bytesRead;
        bytesLeft -= bytesRead;
        buf += bytesRead;
        if(bytesRead != bytesToRead)
            break;
    }
    return n - bytesLeft;
}

int64_t EXT2::write(EXT2::FSINode *inode, const void *buffer, uint64_t position, int64_t n)
{
    size64_t size = inode->GetSize();

    if(position > size)
    {   // writing past end of file; zero padding required
        int64_t newSize = inode->Resize(position);
        if(newSize < 0)
            return newSize;
    }

    time_t curTime = time(nullptr);

    int64_t bytesLeft = n;
    byte *buf = (byte *)buffer;
    while(bytesLeft > 0)
    {
        int64_t blockNum = position / blockSize;
        int64_t blockIdx = getINodeBlock(inode, blockNum);
        if(!blockIdx)
        {
            blockIdx = allocBlock(~0, nullptr); // TODO: add preferred block group to reduce head seek times
            if(!blockIdx)
                break;
            if(!setINodeBlock(inode, blockNum, blockIdx))
            {
                freeBlock(blockIdx);
                break;
            }
            inode->setSize(min((blockNum + 1) * blockSize, inode->Data.i_size + bytesLeft));
            inode->Data.i_blocks += align(blockSize, 512) / 512; // i_blocks uses fixed size 512 byte units
            inode->Dirty = true;
        }

        int64_t inBlockOffset = position % blockSize;
        int64_t blockOffset = blockIdx * blockSize;
        int64_t bytesToWrite = min(bytesLeft, blockSize - inBlockOffset);
        int64_t bytesWritten = Volume->Write(buf, blockOffset + inBlockOffset, bytesToWrite);
        if(bytesWritten < 0)
            return bytesWritten;
        position += bytesWritten;
        inode->SetModifyTime(curTime);
        ((EXT2 *)inode->FS)->superBlock->s_wtime = curTime;
        ((EXT2 *)inode->FS)->superDirty = true;
        inode->setSize(max(position, inode->GetSize()));
        inode->Dirty = true;
        bytesLeft -= bytesWritten;
        buf += bytesWritten;
        if(bytesWritten != bytesToWrite)
            break;
    }
    return n - bytesLeft;
}

uint32_t EXT2::getINodeBlock(FSINode *inode, uint32_t n)
{
    uint32_t blocksPerBlock = blockSize / 4;
    uint32_t square = blocksPerBlock * blocksPerBlock;
    uint32_t cube = square * blocksPerBlock;
    if(n < 12)
        return inode->Data.i_block[n]; // direct
    else if(n < (12 + blocksPerBlock))
    { // singly indirect
        off_t offs = 4 * (n - 12);
        uint32_t r = 0;
        if(Volume->Read(&r, offs + (int64_t)blockSize * inode->Data.i_block[12], 4) < 0)
            return 0;
        return r;
    }
    else if(n < (12 + square))
    { // doubly indirect
        uint32_t r = 0;
        off_t diff = (n - (12 + blocksPerBlock));
        off_t offs = 4 * (diff / blocksPerBlock);
        if(Volume->Read(&r, offs + (int64_t)blockSize * inode->Data.i_block[13], 4) < 0)
            return 0;
        offs = 4 * (diff % blocksPerBlock);
        if(Volume->Read(&r, offs + (int64_t)blockSize * r, 4) < 0)
            return 0;
        return r;
    }
    else if(n < (12 + cube))
    { // triply indirect
        uint32_t r = 0;
        off_t diff = (n - (12 + square));
        off_t offs = 4 * (diff / square);
        if(Volume->Read(&r, offs + (int64_t)blockSize * inode->Data.i_block[14], 4) < 0)
            return 0;
        offs = 4 * ((diff % square) / blocksPerBlock);
        if(Volume->Read(&r, offs + (int64_t)blockSize * r, 4) < 0)
            return 0;
        offs = 4 * (diff / blocksPerBlock);
        if(Volume->Read(&r, offs + (int64_t)blockSize * r, 4) < 0)
            return 0;
        return r;
    }
    return 0;
}

bool EXT2::isBlockZeroed(uint32_t block)
{
    byte buf[16];
    for(uint i = 0; i < blockSize; i += sizeof(buf))
    {
        if(Volume->Read(buf, blockSize * block, sizeof(buf)) != sizeof(buf))
            return false; // we couldn't properly read block data so assume that it is not zeroed
        if(memcmp(buf, blockOfZeros, sizeof(buf)))
            return false;
    }
    return true;
}

bool EXT2::setINodeBlock(FSINode *inode, uint32_t n, uint32_t block)
{
    uint32_t blocksPerBlock = blockSize / 4;
    uint32_t square = blocksPerBlock * blocksPerBlock;
    uint32_t cube = square * blocksPerBlock;
    if(n < 12)
    { // direct
        inode->Data.i_block[n] = block;
        inode->Dirty = true;
        return true;
    }
    else if(n < (12 + blocksPerBlock))
    { // singly indirect
        if(!inode->Data.i_block[12])
        { // allocate 1st level indirect block if needed
            uint32_t l1Blk = allocBlock(~0, nullptr);
            if(!l1Blk) return false;
            if(!zeroBlock(l1Blk))
            {
                freeBlock(l1Blk);
                return false;
            }
            inode->Data.i_block[12] = l1Blk;
            inode->Dirty = true;
        }

        off_t offs = 4 * (n - 12);
        if(Volume->Write(&block, offs + (int64_t)blockSize * inode->Data.i_block[12], 4) < 0)
            return false;
        if(isBlockZeroed(inode->Data.i_block[12]))
        { // free unused indirect block
            freeBlock(inode->Data.i_block[12]);
            inode->Data.i_block[12] = 0;
        }
        return true;
    }
    else if(n < (12 + square))
    { // doubly indirect
        if(!inode->Data.i_block[13])
        { // allocate 1st level indirect block if needed
            uint32_t l1Blk = allocBlock(~0, nullptr);
            if(!l1Blk) return false;
            if(!zeroBlock(l1Blk))
            {
                freeBlock(l1Blk);
                return false;
            }
            inode->Data.i_block[13] = l1Blk;
            inode->Dirty = true;
        }

        uint32_t r = 0;
        off_t diff = (n - (12 + blocksPerBlock));
        off_t offs = 4 * (diff / blocksPerBlock);
        off_t l1offs = offs;
        if(Volume->Read(&r, offs + (int64_t)blockSize * inode->Data.i_block[13], 4) < 0)
            return false;

        if(!r)
        { // allocate 2nd level indirect block if needed
            uint32_t l2Blk = allocBlock(~0, nullptr);
            if(!l2Blk) return false;
            if(!zeroBlock(l2Blk))
            {
                freeBlock(l2Blk);
                return false;
            }
            r = l2Blk;
            if(Volume->Write(&r, offs + (int64_t)blockSize * inode->Data.i_block[13], 4) < 0)
            {
                freeBlock(l2Blk);
                return false;
            }
        }

        offs = 4 * (diff % blocksPerBlock);
        if(Volume->Write(&block, offs + (int64_t)blockSize * r, 4) < 0)
            return false;
        if(isBlockZeroed(r))
        { // free unused indirect block
            freeBlock(r);
            if(Volume->Write(blockOfZeros, l1offs + (int64_t)blockSize * inode->Data.i_block[13], 4) < 0)
                return false;
        }
        if(isBlockZeroed(inode->Data.i_block[13]))
        { // free unused indirect block
            freeBlock(inode->Data.i_block[13]);
            inode->Data.i_block[13] = 0;
        }
        return true;
    }
    else if(n < (12 + cube))
    { // triply indirect
        if(!inode->Data.i_block[14])
        { // allocate 1st level indirect block if needed
            uint32_t l1Blk = allocBlock(~0, nullptr);
            if(!l1Blk) return false;
            if(!zeroBlock(l1Blk))
            {
                freeBlock(l1Blk);
                return false;
            }
            inode->Data.i_block[14] = l1Blk;
            inode->Dirty = true;
        }

        uint32_t r = 0;
        off_t diff = (n - (12 + square));
        off_t offs = 4 * (diff / square);
        off_t l1offs = offs;
        if(Volume->Read(&r, offs + (int64_t)blockSize * inode->Data.i_block[14], 4) < 0)
            return false;

        uint32_t l2Blk = 0;
        if(!r)
        { // allocate 2nd level indirect block if needed
            l2Blk = allocBlock(~0, nullptr);
            if(!l2Blk) return false;
            if(!zeroBlock(l2Blk))
            {
                freeBlock(l2Blk);
                return false;
            }
            r = l2Blk;
            if(Volume->Write(&r, offs + (int64_t)blockSize * inode->Data.i_block[14], 4) < 0)
            {
                freeBlock(l2Blk);
                return false;
            }
        }

        offs = 4 * ((diff % square) / blocksPerBlock);
        off_t l2offs = offs;
        uint32_t r2 = r;
        if(Volume->Read(&r, offs + (int64_t)blockSize * r, 4) < 0)
            return false;

        uint32_t l3Blk = 0;
        if(!r)
        { // allocate 3rd level indirect block if needed
            l3Blk = allocBlock(~0, nullptr);
            if(!l3Blk) return false;
            if(!zeroBlock(l3Blk))
            {
                freeBlock(l3Blk);
                return false;
            }
            r = l3Blk;
            if(Volume->Write(&r, offs + (int64_t)blockSize * r2, 4) < 0)
            {
                freeBlock(l3Blk);
                return false;
            }
        }

        offs = 4 * (diff / blocksPerBlock);
        if(Volume->Write(&block, offs + (int64_t)blockSize * r, 4) < 0)
            return false;

        if(isBlockZeroed(l3Blk))
        { // free unused indirect block
            freeBlock(l3Blk);
            if(Volume->Write(blockOfZeros, l2offs + (int64_t)blockSize * l2Blk, 4) < 0)
                return false;
        }
        if(isBlockZeroed(l2Blk))
        { // free unused indirect block
            freeBlock(l2Blk);
            if(Volume->Write(blockOfZeros, l1offs + (int64_t)blockSize * inode->Data.i_block[14], 4) < 0)
                return false;
        }
        if(isBlockZeroed(inode->Data.i_block[14]))
        { // free unused indirect block
            freeBlock(inode->Data.i_block[14]);
            inode->Data.i_block[14] = 0;
        }
        return true;
    }
    return false;
}

bool EXT2::zeroBlock(uint32_t block)
{
    if(Volume->Write(blockOfZeros, (int64_t)blockSize * block, blockSize) != blockSize)
        return false;
    return true;
}

EXT2::~EXT2()
{
    Lock();
    if(Root) PutDEntry(Root);
    WriteSuperBlock();
    Volume->Flush();
    delete superBlock;
    delete[] BGDT;
}

bool EXT2::GetLabel(char *buffer, size_t num)
{
    if(!Lock()) return false;
    if(!superBlock->s_volume_name[0])
    {
        UnLock();
        return false;
    }
    memset(buffer, 0, num);
    strncpy(buffer, superBlock->s_volume_name, min(16, num));
    UnLock();
    return true;
}

UUID EXT2::GetUUID()
{
    if(!Lock()) return UUID::nil;
    UUID uuid(superBlock->s_uuid);
    UnLock();
    return uuid;
}

::INode *EXT2::ReadINode(ino_t number)
{
    if(!Lock() || !initialized) return nullptr;
    FSINode *inode = new FSINode(number, this);
    int64_t btr = min(sizeof(EXT2::INode), superBlock->s_inode_size);
    int64_t br = Volume->Read(&inode->Data, getINodeOffset(number), btr);
    if(br != btr)
    {
        delete inode;
        UnLock();
        return nullptr;
    }
    UnLock();
    return inode;
}

bool EXT2::WriteINode(::INode *inode)
{
    if(!Lock() || !initialized) return false;
    FSINode *i = (FSINode *)inode;
    int64_t btw = min(sizeof(EXT2::INode), superBlock->s_inode_size);
    int64_t bw = Volume->Write(&i->Data, getINodeOffset(inode->Number), btw);
    UnLock();
    return bw == btw;
}

bool EXT2::WriteSuperBlock()
{
    if(!Lock() || !initialized) return false;
    if(!superDirty)
    {
        UnLock();
        return true;
    }
    int64_t bw = Volume->Write(superBlock, 1024, sizeof(SuperBlock));
    if(bw != sizeof(SuperBlock))
    {
        printf("[ext2] ext2SuperBlockWriteSuper: couldn't write main\n"
               "       superblock on volume %d. Use fsck.\n", Volume->ID);
        UnLock();
        return false;
    }
    for(uint i = 0; i < blockGroupCount; ++i)
    {
        if(!hasSuperBlock(i))
            continue;
        uint64_t offs = blockGroupOffset(i);
        if(offs <= 1024) continue;
        bw = Volume->Write(superBlock, offs, sizeof(SuperBlock));
        if(bw != sizeof(SuperBlock))
        {
            printf("[ext2] ext2SuperBlockWriteSuper: couldn't write backup\n"
                   "       superblock (group %d) on volume %d. Use fsck.\n", i, Volume->ID);
            UnLock();
            return false;
        }
    }
    superDirty = false;
    UnLock();
    return true;
}

EXT2::FSINode::FSINode(ino_t number, FileSystem *fs) :
    ::INode(number, fs)
{
}

void EXT2::FSINode::setSize(size64_t size)
{
    if(!INode::Lock()) return;
    if(!FileSystem::Lock())
    {
        INode::UnLock();
        return;
    }

    if(((EXT2 *)FS)->superBlock->s_rev_level < 1)
        Data.i_size = size;
    else
    {
        if(EXT2_S_ISREG(Data.i_mode))
            Data.i_dir_acl = size >> 32;
        Data.i_size = size & 0xFFFFFFFF;
    }
    Dirty = true;

    FileSystem::UnLock();
    INode::UnLock();
}

bool EXT2::FSINode::buildDirectory(EXT2::FSINode *parentINode, FSINode *newINode)
{
    if(!INode::Lock())
        return false;
    if(!FileSystem::Lock())
    {
        INode::UnLock();
        return false;
    }
    EXT2 *fs = (EXT2 *)newINode->FS;
    size_t blockSize = fs->blockSize;
    FileSystem::UnLock();

    if(!newINode->Resize(blockSize))
    {
        INode::UnLock();
        return false;
    }

    struct DirInitializer
    {
        DirectoryEntry dotDE = { 0, sizeof(EXT2::DirectoryEntry) + sizeof(DirInitializer::dot), 1, EXT2_FT_DIR};
        char dot[4] = { '.' };
        DirectoryEntry dotDotDE = { 0, sizeof(EXT2::DirectoryEntry) + sizeof(DirInitializer::dotDot), 2, EXT2_FT_DIR};
        char dotDot[4] = { '.', '.' };
    } dirInitializer;
    dirInitializer.dotDE.inode = newINode->Number;
    dirInitializer.dotDotDE.inode = parentINode->Number;
    dirInitializer.dotDotDE.rec_len = blockSize - offsetof(DirInitializer, dotDotDE);

    if(newINode->Write(&dirInitializer, 0, sizeof(dirInitializer)) != sizeof(dirInitializer))
    {
        INode::UnLock();
        return false;
    }

    ++parentINode->Data.i_links_count;
    parentINode->Dirty = true;
    ++newINode->Data.i_links_count;
    newINode->Dirty = true;

    INode::UnLock();
    return true;
}

size64_t EXT2::FSINode::GetSize()
{
    if(!INode::Lock()) return 0;
    if(!FileSystem::Lock())
    {
        INode::UnLock();
        return 0;
    }
    FSINode *inode = (FSINode *)this;
    size64_t size = ((EXT2 *)FS)->superBlock->s_rev_level < 1 ?
                inode->Data.i_size :
                (inode->Data.i_size | ((inode->Data.i_mode & 0xF000) == EXT2_S_IFREG ?
                                           ((uint64_t)inode->Data.i_dir_acl << 32) :
                                           0));
    FileSystem::UnLock();
    INode::UnLock();
    return size;
}

mode_t EXT2::FSINode::GetMode()
{
    if(!INode::Lock()) return 0;
    time_t res = Data.i_mode;
    INode::UnLock();
    return res;
}

time_t EXT2::FSINode::GetCreateTime()
{
    if(!INode::Lock()) return 0;
    time_t res = Data.i_ctime;
    INode::UnLock();
    return res;
}

time_t EXT2::FSINode::GetModifyTime()
{
    if(!INode::Lock()) return 0;
    time_t res = Data.i_mtime;
    INode::UnLock();
    return res;
}

time_t EXT2::FSINode::GetAccessTime()
{
    if(!INode::Lock()) return 0;
    time_t res = Data.i_atime;
    INode::UnLock();
    return res;
}

bool EXT2::FSINode::SetCreateTime(time_t t)
{
    if(!INode::Lock()) return false;
    Data.i_ctime = t;
    Dirty = true;
    INode::UnLock();
    return true;
}

bool EXT2::FSINode::SetModifyTime(time_t t)
{
    if(!INode::Lock()) return false;
    Data.i_mtime = t;
    Dirty = true;
    INode::UnLock();
    return true;
}

bool EXT2::FSINode::SetAccessTime(time_t t)
{
    if(!INode::Lock()) return false;
    Data.i_atime = t;
    Dirty = true;
    INode::UnLock();
    return true;
}

bool EXT2::FSINode::Create(const char *name, mode_t mode)
{
    if(!isValidFileName(name))
        return false;
    int nameLen = strlen(name);
    DirectoryEntry de = { 0, (uint16_t)align(sizeof(DirectoryEntry) + nameLen, 4),
                          (uint8_t)nameLen, 0 };
    DirectoryEntry cde;
    int64_t readRes = 0, position = 0;

    if(!INode::Lock())
        return false;

    if(!EXT2_S_ISDIR(Data.i_mode))
    {   // we can't create files in non-dirs
        INode::UnLock();
        return false;
    }

    // check if file/directory with that name already exists
    if(Lookup(name))
    {
        INode::UnLock();
        return false;
    }

    // get fs revision, new inode, upd, gid and block size
    if(!FileSystem::Lock())
    {
        INode::UnLock();
        return false;
    }
    EXT2 *fs = (EXT2 *)FS;
    size_t blockSize = fs->blockSize;
    uint32_t fsRev = fs->superBlock->s_rev_level;
    uint32_t bg = 0;
    uint32_t ino = fs->allocINode(&bg);
    if(!ino)
    {
        INode::UnLock();
        return false;
    }
    Thread *ct = Thread::GetCurrent();
    if(!ino || !ct || !ct->Process)
    {
        INode::UnLock();
        FileSystem::UnLock();
        return false;
    }
    uid_t uid = ct->Process->UID;
    gid_t gid = ct->Process->GID;

    FSINode *inode = (FSINode *)fs->GetINode(ino);
    if(!inode)
    {
        FileSystem::UnLock();
        INode::UnLock();
        return false;
    }
    inode->Data.i_mode = mode;
    inode->Data.i_uid = uid;
    inode->Data.i_ctime = time(nullptr);
    inode->Data.i_mtime = inode->Data.i_ctime;
    inode->Data.i_dtime = 0;
    inode->Data.i_gid = gid;
    inode->Data.i_links_count = 1;
    inode->Dirty = true;

    de.inode = ino;
    if(fsRev >= 1) de.file_type = modeToFileType(mode);

    // look for empty entry
    readRes = 0;
    position = 0;
    while((readRes = Read(&cde, position, sizeof(cde))) == sizeof(cde))
    {
        bool firstInBlock = !(position % blockSize);
        size_t cdeMinSize = align(sizeof(cde) + cde.name_len, 4);
        int64_t cdeMinEnd = position + cdeMinSize;
        int64_t origCdeEnd = position + cde.rec_len;
        if((cde.rec_len - cdeMinSize) >= de.name_len)
        {   // new entry will fit here

            if(!firstInBlock)
            {   // shrink current entry
                cde.rec_len = cdeMinSize;
                if(Write(&cde, position, sizeof(cde)) != sizeof(cde))
                {
                    fs->PutINode(inode);
                    FileSystem::UnLock();
                    INode::UnLock();
                    return false;
                }
            }

            // write new entry
            de.rec_len = origCdeEnd - cdeMinEnd;
            if(Write(&de, cdeMinEnd, sizeof(de)) != sizeof(de))
            {
                fs->PutINode(inode);
                INode::UnLock();
                return false;
            }

            // write new filename
            if(Write(name, cdeMinEnd + sizeof(de), nameLen) != nameLen)
            {
                fs->PutINode(inode);
                FileSystem::UnLock();
                INode::UnLock();
                return false;
            }
            if(EXT2_S_ISDIR(mode))
            {
                buildDirectory(this, inode);
                ++fs->BGDT[bg].bg_used_dirs_count;
                fs->updateBGDT(bg, offsetof(BlockGroupDescriptor, bg_used_dirs_count), 2);
            }
            fs->PutINode(inode);
            FileSystem::UnLock();
            INode::UnLock();
            return true;
        }


        if(!cde.inode && firstInBlock)
        {   // unused entry (only valid at the start of a block)
            if(cde.rec_len >= de.rec_len)
            {   // new entry will fit in here
                de.rec_len = cde.rec_len;
                if(Write(&de, position, sizeof(de)) != sizeof(de))
                {
                    fs->PutINode(inode);
                    FileSystem::UnLock();
                    INode::UnLock();
                    return false;
                }
                if(Write(name, position + sizeof(de), nameLen) != nameLen)
                {
                    INode::UnLock();
                    return false;
                }
                if(EXT2_S_ISDIR(mode))
                {
                    buildDirectory(this, inode);
                    ++fs->BGDT[bg].bg_used_dirs_count;
                    fs->updateBGDT(bg, offsetof(BlockGroupDescriptor, bg_used_dirs_count), 2);
                }
                fs->PutINode(inode);
                FileSystem::UnLock();
                INode::UnLock();
                return true;
            }
        }
        position += cde.rec_len;
    }

    // new block must be allocated
    size64_t cSize = align(GetSize(), blockSize);
    size64_t newSize = cSize + blockSize;
    if(Resize(newSize) != newSize)
    {
        fs->PutINode(inode);
        FileSystem::UnLock();
        INode::UnLock();
        return false;
    }
    de.rec_len = blockSize;
    if(Write(&de, cSize, sizeof(de)) != sizeof(de))
    {
        fs->PutINode(inode);
        FileSystem::UnLock();
        INode::UnLock();
        return false;
    }
    if(Write(name, cSize + sizeof(de), nameLen) != nameLen)
    {
        fs->PutINode(inode);
        FileSystem::UnLock();
        INode::UnLock();
        return false;
    }
    if(EXT2_S_ISDIR(mode))
    {
        buildDirectory(this, inode);
        ++fs->BGDT[bg].bg_used_dirs_count;
        fs->updateBGDT(bg, offsetof(BlockGroupDescriptor, bg_used_dirs_count), 4);
    }
    fs->PutINode(inode);
    FileSystem::UnLock();
    INode::UnLock();
    return true;
}

ino_t EXT2::FSINode::Lookup(const char *name)
{
    if(!INode::Lock())
        return -EBUSY;
    if(!(Data.i_mode & EXT2_S_IFDIR))
    { // we can't look for files inside non-directory inode
        INode::UnLock();
        return -EBUSY;
    }
    uint64_t pos = 0;
    size64_t size = GetSize();
    DirectoryEntry de;
    char nameBuf[256];
    while(pos < size)
    {
        if(Read(&de, pos, sizeof(DirectoryEntry)) != sizeof(DirectoryEntry))
        {
            INode::UnLock();
            return -EIO;
        }

        if(de.name_len && de.inode)
        {
            memset(nameBuf, 0, sizeof(nameBuf));
            if(Read(nameBuf, pos + sizeof(DirectoryEntry), de.name_len) != de.name_len)
            {
                INode::UnLock();
                return -EIO;
            }

            if(!strcmp(name, nameBuf))
            { // found
                INode::UnLock();
                return de.inode;
            }
        }

        pos += de.rec_len;
    }
    INode::UnLock();
    return 0;
}

int64_t EXT2::FSINode::Read(void *buffer, int64_t position, int64_t n)
{
    if(!Lock()) return -EBUSY;
    if(!FileSystem::Lock())
    {
        UnLock();
        return -EBUSY;
    }
    int64_t res = ((EXT2 *)FS)->read(this, buffer, position, n);
    FileSystem::UnLock();
    UnLock();
    return res;
}

int64_t EXT2::FSINode::Write(const void *buffer, int64_t position, int64_t n)
{
    if(!Lock()) return -EBUSY;
    if(!FileSystem::Lock())
    {
        UnLock();
        return -EBUSY;
    }
    int64_t res = ((EXT2 *)FS)->write(this, buffer, position, n);
    FileSystem::UnLock();
    UnLock();
    return res;
}

::DirectoryEntry *EXT2::FSINode::ReadDir(int64_t position, int64_t *newPosition)
{
    if(!INode::Lock()) return nullptr;
    if(!(Data.i_mode & EXT2_S_IFDIR))
    { // we can't do ReadDir for non-directory
        INode::UnLock();
        return nullptr;
    }

    EXT2::DirectoryEntry de;
    char nameBuf[256];
    size64_t size = GetSize();
    ::DirectoryEntry *res = nullptr;

    while(position < size)
    {
        if(Read(&de, position, sizeof(DirectoryEntry)) != sizeof(DirectoryEntry))
            break;

        if(!de.inode || !de.name_len)
        {
            position += de.rec_len;
            continue;
        }

        memset(nameBuf, 0, sizeof(nameBuf));
        if(Read(nameBuf, position + sizeof(DirectoryEntry), de.name_len) != de.name_len)
            break;

        ::INode *inode = FS->ReadINode(de.inode);
        if(!inode)
            break;

        res = new ::DirectoryEntry(
                    inode->GetMode(),
                    inode->GetAccessTime(),
                    inode->GetCreateTime(),
                    inode->GetModifyTime(),
                    inode->GetSize(),
                    inode->Number,
                    nameBuf);

        delete inode;

        position += de.rec_len;
        break;
    }

    if(newPosition)
        *newPosition = position;
    INode::UnLock();
    return res;
}

int64_t EXT2::FSINode::Resize(int64_t size)
{
    if(!INode::Lock())
        return -EBUSY;
    if(!FileSystem::Lock())
    {
        INode::UnLock();
        return -EBUSY;
    }
    EXT2 *fs = (EXT2 *)FS;
    time_t curTime = time(nullptr);

    size_t currentSize = GetSize();
    if(size < currentSize)
    {  // shrink file
        uint32_t startBlk = align(size, fs->blockSize) / fs->blockSize;
        size_t blkCount = align(currentSize, fs->blockSize) / fs->blockSize;
        for(uint32_t blk = startBlk; blk < blkCount; ++blk)
        {
            uint32_t blkIdx = fs->getINodeBlock(this, blk);
            if(!blkIdx)
            {
                printf("[ext2] Something went wrong in FSINode::Resize()\n"
                       "       Filesystem may be inconsistent. Run fsck.\n");
                INode::UnLock();
                FileSystem::UnLock();
                return -EIO;
            }
            fs->freeBlock(blkIdx);
            fs->setINodeBlock(this, blk, 0);
        }
        Data.i_blocks = align(size, 512) / 512;
        setSize(size);
    }
    else if(size > currentSize)
    { // grow file
        int64_t bytesLeft = size - currentSize;
        int64_t curPos = currentSize;
        while(bytesLeft > 0)
        {
            int64_t blockNum = curPos / fs->blockSize;
            int64_t blockIdx = fs->getINodeBlock(this, blockNum);
            if(!blockIdx)
            {
                blockIdx = fs->allocBlock(~0, nullptr); // TODO: add preferred block group to reduce head seek times
                if(!blockIdx)
                    break;
                if(!fs->setINodeBlock(this, blockNum, blockIdx))
                {
                    fs->freeBlock(blockIdx);
                    break;
                }
                setSize(min((blockNum + 1) * fs->blockSize, Data.i_size + bytesLeft));
                Data.i_blocks += align(fs->blockSize, 512) / 512; // i_blocks uses fixed size 512 byte units
                Dirty = true;
            }
            int64_t inBlockOffset = curPos % fs->blockSize;
            int64_t blockOffset = blockIdx * fs->blockSize;
            int64_t bytesToWrite = min(bytesLeft, fs->blockSize - inBlockOffset);
            int64_t bytesWritten = fs->Volume->Write(fs->blockOfZeros, blockOffset + inBlockOffset, bytesToWrite);
            if(bytesWritten < 0)
            {
                FileSystem::UnLock();
                INode::UnLock();
                return bytesWritten;
            }
            curPos += bytesWritten;
            SetModifyTime(curTime);
            fs->superBlock->s_wtime = curTime;
            fs->superDirty = true;
            setSize(max(curPos, GetSize()));
            Dirty = true;
            bytesLeft -= bytesWritten;
            if(bytesWritten != bytesToWrite)
            {
                FileSystem::UnLock();
                INode::UnLock();
                return -EIO;
            }
        }
    }
    currentSize = GetSize();
    FileSystem::UnLock();
    INode::UnLock();
    return currentSize;
}

int EXT2::FSINode::Remove(const char *name)
{
    if(!INode::Lock())
        return -EBUSY;

    if(!(Data.i_mode & EXT2_S_IFDIR))
    { // there are no files in non-directory
        INode::UnLock();
        return -ENOTDIR;
    }

    EXT2::DirectoryEntry de, prevDe;
    char nameBuf[256];
    size64_t size = GetSize();
    int64_t position = 0, prevPos = 0;
    while(position < size)
    {
        if(Read(&de, position, sizeof(de)) != sizeof(de) || de.rec_len < sizeof(de))
        {
            INode::UnLock();
            return -EIO;
        }

        if(!de.inode || !de.name_len)
        {   // skip empty entry
            prevDe = de;
            prevPos = position;
            position += max(de.rec_len, sizeof(de));
            continue;
        }

        // read filename
        memset(nameBuf, 0, sizeof(nameBuf));
        if(Read(nameBuf, position + sizeof(DirectoryEntry), de.name_len) != de.name_len)
        {
            INode::UnLock();
            return -EIO;
        }

        if(strcmp(name, nameBuf))
        {   // check next entry
            prevDe = de;
            prevPos = position;
            position += de.rec_len;
            continue;
        }

        EXT2 *fs = (EXT2 *)FS;
        FSINode *inode = (FSINode *)fs->GetINode(de.inode);
        if(!inode)
        {
            INode::UnLock();
            return -EIO;
        }

        if(!FileSystem::Lock())
        {
            INode::UnLock();
            return -EBUSY;
        }
        size_t blockSize = ((EXT2 *)FS)->blockSize;
        FileSystem::UnLock();


        if(EXT2_S_ISDIR(inode->GetMode()))
        {   // directory
            // check if directory is empty (only . and ..  entries are allowed)
            EXT2::DirectoryEntry de;
            char nameBuf[4];
            size64_t size = inode->GetSize();
            int64_t position = 0;
            bool isEmpty = true;
            uint32_t selfLnkCnt = 0;
            uint32_t parentLnkCnt = 0;
            while(position < size)
            {
                if(inode->Read(&de, position, sizeof(de)) != sizeof(de) || de.rec_len < sizeof(de))
                {
                    INode::UnLock();
                    return -EIO;
                }

                if(!de.inode || !de.name_len)
                {   // skip empty entry
                    position += max(de.rec_len, sizeof(de));
                    continue;
                }

                // read filename
                memset(nameBuf, 0, sizeof(nameBuf));
                int64_t btr = min(sizeof(nameBuf), de.name_len);
                if(inode->Read(nameBuf, position + sizeof(DirectoryEntry), btr) != btr)
                {
                    INode::UnLock();
                    return -EIO;
                }

                if(!strncmp(".", nameBuf, sizeof(nameBuf)))
                {
                    ++selfLnkCnt;
                    position += max(de.rec_len, sizeof(de));
                    continue;
                }

                if(!strncmp("..", nameBuf, sizeof(nameBuf)))
                {
                    ++parentLnkCnt;
                    position += max(de.rec_len, sizeof(de));
                    continue;
                }

                isEmpty = false;
                break;
            }

            if(!isEmpty)
            {
                INode::UnLock();
                return -ENOTEMPTY;
            }

            // update link counts
            inode->Data.i_links_count -= min(inode->Data.i_links_count, selfLnkCnt);
            Data.i_links_count -= min(Data.i_links_count, parentLnkCnt);
            inode->Dirty = true;
            Dirty = true;
        }

        if(inode->Data.i_links_count)
        {
            --inode->Data.i_links_count;
            inode->Dirty = true;
        }

        if(!inode->Data.i_links_count)
        {   // erase file contents
            if(int64_t newSize = inode->Resize(0))
            {
                INode::UnLock();
                return newSize < 0 ? newSize : -EIO;
            }
        }
        FS->PutINode(inode);

        // clear directory entry
        if(!(position % blockSize))
        {   // first entry in a block
            de.inode = 0;
            if(Write(&de, position, sizeof(de)) != sizeof(de))
            {
                INode::UnLock();
                return -EIO;
            }
        }
        else
        {
            prevDe.rec_len += de.rec_len;
            if(Write(&prevDe, prevPos, sizeof(prevDe)) != sizeof(prevDe))
            {
                INode::UnLock();
                return -EIO;
            }
        }

        INode::UnLock();
        return 0;
    }

    INode::UnLock();
    return -ENOENT;
}

int EXT2::FSINode::Release()
{
    if(!INode::Lock())
        return -EBUSY;
    if(!Data.i_links_count)
    {
        Data.i_dtime = time(nullptr);
        if(!FileSystem::Lock())
        {
            INode::UnLock();
            return -EBUSY;
        }        
        EXT2 *fs = (EXT2 *)FS;
        fs->WriteINode(this);
        fs->freeINode(Number);
        if(EXT2_S_ISDIR(Data.i_mode))
        {
            uint32_t bg = fs->getINodeBlockGroup(Number);
            if(fs->BGDT[bg].bg_used_dirs_count)
                --fs->BGDT[bg].bg_used_dirs_count;
            fs->updateBGDT(bg, offsetof(BlockGroupDescriptor, bg_used_dirs_count), 2);
        }
        FileSystem::UnLock();
    }
    INode::UnLock();
    return 0;
}
