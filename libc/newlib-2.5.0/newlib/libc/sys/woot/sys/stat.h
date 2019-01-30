#ifndef STAT_H
#define STAT_H

// make sure this stuff matches to kernel

#define S_IFTYPE 0xF000     // type mask
#define S_IFMT   S_IFTYPE   // same thing as S_IFTYPE
#define S_IFSOCK 0xC000     // socket
#define S_IFLNK  0xA000     // symbolic link
#define S_IFREG  0x8000     // regular file
#define S_IFBLK  0x6000     // block device
#define S_IFDIR  0x4000     // directory
#define S_IFCHR  0x2000     // character device
#define S_IFIFO  0x1000     // fifo

#define S_ISUID  0x0800     // Set process User ID
#define S_ISGID  0x0400     // Set process Group ID
#define S_ISVTX  0x0200     // sticky bit

#define S_IRUSR  0x0100     // user read
#define S_IWUSR  0x0080     // user write
#define S_IXUSR  0x0040     // user execute
#define S_IRGRP  0x0020     // group read
#define S_IWGRP  0x0010     // group write
#define S_IXGRP  0x0008     // group execute
#define S_IROTH  0x0004     // others read
#define S_IWOTH  0x0002     // others write
#define S_IXOTH  0x0001     // others execute

#define S_ISDIR(mode)	(((mode) & S_IFTYPE) == S_IFDIR)
#define S_ISLINK(mode)	(((mode) & S_IFTYPE) == S_IFLNK)
#define S_ISBLK(mode)	(((mode) & S_IFTYPE) == S_IFBLK)
#define S_ISSOCK(mode)	(((mode) & S_IFTYPE) == S_IFSOCK)
#define S_ISREG(mode)	(((mode) & S_IFTYPE) == S_IFREG)
#define S_ISCHAR(mode)	(((mode) & S_IFTYPE) == S_IFCHR)
#define S_ISCHR         S_ISCHAR    // Again, the same thing.
#define S_ISFIFO(mode)	(((mode) & S_IFTYPE) == S_IFIFO)

struct stat
{
    unsigned long st_dev;
    unsigned long st_ino;
    unsigned short st_mode;
    unsigned short st_nlink;
    unsigned short st_uid;
    unsigned short st_gid;
    unsigned long st_rdev;
    unsigned long st_size;
    unsigned long st_blksize;
    unsigned long st_blocks;
    unsigned long st_atime;
    unsigned long st_atime_nsec;
    unsigned long st_mtime;
    unsigned long st_mtime_nsec;
    unsigned long st_ctime;
    unsigned long st_ctime_nsec;
    unsigned long __unused4;
    unsigned long __unused5;
};

#define stat64 stat

#endif // STAT_H
