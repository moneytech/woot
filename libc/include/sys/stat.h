#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <sys/types.h>
#include <time.h>

#define S_IFTYPE 0xF000 // type mask
#define S_IFSOCK 0xC000 // socket
#define S_IFLNK  0xA000 // symbolic link
#define S_IFREG  0x8000 // regular file
#define S_IFBLK  0x6000 // block device
#define S_IFDIR  0x4000 // directory
#define S_IFCHR  0x2000 // character device
#define S_IFIFO  0x1000 // fifo

#define S_ISUID  0x0800 // user id bit
#define S_ISGID  0x0400 // group id bit
#define S_ISVTX  0x0200 // sticky bit

#define S_IRUSR  0x0100 // user read
#define S_IWUSR  0x0080 // user write
#define S_IXUSR  0x0040 // user execute
#define S_IRGRP  0x0020 // group read
#define S_IWGRP  0x0010 // group write
#define S_IXGRP  0x0008 // group execute
#define S_IROTH  0x0004 // others read
#define S_IWOTH  0x0002 // others write
#define S_IXOTH  0x0001 // others execute

#define S_ISDIR(mode)	(((mode) & S_IFTYPE) == S_IFDIR)
#define S_ISLINK(mode)	(((mode) & S_IFTYPE) == S_IFLNK)
#define S_ISBLK(mode)	(((mode) & S_IFTYPE) == S_IFBLK)
#define S_ISSOCK(mode)	(((mode) & S_IFTYPE) == S_IFSOCK)
#define S_ISREG(mode)	(((mode) & S_IFTYPE) == S_IFREG)
#define S_ISCHAR(mode)	(((mode) & S_IFTYPE) == S_IFCHR)
#define S_ISFIFO(mode)	(((mode) & S_IFTYPE) == S_IFIFO)

struct stat
{
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    size_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    time_t st_atime;
    unsigned long st_atime_nsec;
    time_t st_mtime;
    unsigned long st_mtime_nsec;
    time_t st_ctime;
    unsigned long st_ctime_nsec;
    unsigned long __unused4;
    unsigned long __unused5;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int stat(const char *path, struct stat *buf);
int mkdir(const char *pathname, mode_t mode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SYS_STAT_H
