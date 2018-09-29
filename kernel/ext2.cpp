#include <dentry.h>
#include <errno.h>
#include <ext2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volume.h>

byte EXT2::zero1k[1024]; // 1 kiB of zeros

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
    FileSystem::Add(fs);

    vol->UnLock();
    delete sblock;
    return nullptr;
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
    initialized(false), superDirty(false)
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
        return (g * superBlock->s_blocks_per_group) + (j * 8 + bit);
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
    return true;
}

int64_t EXT2::read(FSINode *inode, void *buffer, uint64_t position, int64_t n)
{
    size64_t size = inode->GetSize();
    if((position + n) > size)
        n = size - position;
    if(!n) return 0;

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
        int64_t bRead = Volume->Read(buf, blockOffset + inBlockOffset, bytesToRead);
        if(bRead < 0)
            return bRead;
        position += bRead;
        bytesLeft -= bRead;
        buf += bRead;
        if(bRead != bytesToRead)
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

bool EXT2::setINodeBlock(FSINode *inode, uint32_t n, uint32_t block)
{
    uint32_t blocksPerBlock = blockSize / 4;
    uint32_t square = blocksPerBlock * blocksPerBlock;
    uint32_t cube = square * blocksPerBlock;
    if(n < 12)
    { // direct
        inode->Data.i_block[n] = block;
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
        }

        off_t offs = 4 * (n - 12);
        if(Volume->Write(&block, offs + (int64_t)blockSize * inode->Data.i_block[12], 4) < 0)
            return false;
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
        }

        uint32_t r = 0;
        off_t diff = (n - (12 + blocksPerBlock));
        off_t offs = 4 * (diff / blocksPerBlock);
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
        }

        uint32_t r = 0;
        off_t diff = (n - (12 + square));
        off_t offs = 4 * (diff / square);
        if(Volume->Read(&r, offs + (int64_t)blockSize * inode->Data.i_block[14], 4) < 0)
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
            if(Volume->Write(&r, offs + (int64_t)blockSize * inode->Data.i_block[14], 4) < 0)
            {
                freeBlock(l2Blk);
                return false;
            }
        }

        offs = 4 * ((diff % square) / blocksPerBlock);
        uint32_t r2 = r;
        if(Volume->Read(&r, offs + (int64_t)blockSize * r, 4) < 0)
            return false;

        if(!r)
        { // allocate 3rd level indirect block if needed
            uint32_t l3Blk = allocBlock(~0, nullptr);
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
        return true;
    }
    return false;
}

bool EXT2::zeroBlock(uint32_t block)
{
    for(uint i = 0; i < superBlock->s_log_block_size; ++i)
    {
        if(Volume->Write(zero1k, (int64_t)blockSize * block, 1024) != 1024)
            return false;
    }
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
    int64_t btw = min(sizeof(EXT2::INode), superBlock->s_inode_size);
    int64_t bw = Volume->Write(&((FSINode *)inode)->Data, getINodeOffset(inode->Number), btw);
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

ino_t EXT2::FSINode::Lookup(const char *name)
{
    if(!INode::Lock()) return -EBUSY;
    if(!FileSystem::Lock())
    {
        INode::UnLock();
        return -EBUSY;
    }
    if(!(Data.i_mode & EXT2_S_IFDIR))
    { // we can't look for files inside non-directory inode
        INode::UnLock();
        FileSystem::UnLock();
        return -1;
    }
    EXT2 *fs = (EXT2 *)this->FS;
    uint64_t pos = 0;
    size64_t size = GetSize();
    DirectoryEntry de;
    char nameBuf[256];
    while(pos < size)
    {
        if(fs->read(this, &de, pos, sizeof(DirectoryEntry)) != sizeof(DirectoryEntry))
        {
            INode::UnLock();
            FileSystem::UnLock();
            return -1;
        }

        if(de.name_len)
        {
            memset(nameBuf, 0, sizeof(nameBuf));
            if(fs->read(this, nameBuf, pos + sizeof(DirectoryEntry), de.name_len) != de.name_len)
            {
                INode::UnLock();
                FileSystem::UnLock();
                return -1;
            }

            if(!strcmp(name, nameBuf))
            { // found
                INode::UnLock();
                FileSystem::UnLock();
                return de.inode;
            }
        }

        pos += de.rec_len;
    }
    INode::UnLock();
    FileSystem::UnLock();
    return -1;
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
    return -ENOSYS;
}
