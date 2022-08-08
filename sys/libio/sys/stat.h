/* sys/stat.h
 *
 * POSIX stat.h stuff, reference:
 * https://pubs.opengroup.org/onlinepubs/009696899/basedefs/sys/stat.h.html */

#ifndef __SYS_STAT_H__
#define __SYS_STAT_H__ 1

#include <sys/types.h>

struct stat {
    dev_t st_dev; /* Device id */
    ino_t st_ino; /* Serial number */
    mode_t st_mode; /* Mode of file */
    nlink_t st_nlink; /* Number of hard links */
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev; /* Device ID if file is char or block */
    off_t st_size; /* Size of file for normal files, for symbolic links the length of the path */
    time_t st_atime; /* Time of access */
    time_t st_mtime; /* Time of modification */
    time_t st_ctime; /* Time of last status change */
    blksize_t st_blksize; /* Block size */
    blkcnt_t st_blocks; /* Number of blocks */
};

#define S_ISBLK(m) 0ul /* Test for blockfile */
#define S_ISCHR(m) 0ul /* Test for charfile */
#define S_ISDIR(m) 0ul /* Test for directory */
#define S_ISFIFO(m) 0ul /* Test for FIFO */
#define S_ISREG(m) 1ul /* Test for file */
#define S_ISLNK(m) 0ul /* Test for symbolic link */
#define S_ISSOCK(m) 0ul /* Test for socket */

int stat(const char *name, struct stat *st);
int fstat(int fd, struct stat *st);
int lstat(const char *name, struct stat *st);

int chmod(const char *name, mode_t mode);
int fchmod(int fd, mode_t mode);
int mkdir(const char *name, mode_t mode);
int mkfifo(const char *name, mode_t mode);
int mknod(const char *name, mode_t mode, dev_t dev);
mode_t umask(mode_t mode);

#endif
