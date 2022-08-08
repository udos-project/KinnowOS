#ifndef __LIBIO_FCNTL_H__
#define __LIBIO_FCNTL_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

/* mode_t */
#include <sys/types.h>

/* Constants reference for FCNTL.H:
 * https://pubs.opengroup.org/onlinepubs/9699919799.2016edition/functions/open.html
 * https://pubs.opengroup.org/onlinepubs/9699919799.2016edition/basedefs/fcntl.h.html
 */

#define F_DUPFD 0x01 /* Duplicate file descriptor */
#define F_DUPFD_CLOEXEC 0x02 /*  Duplicate file descriptor with the close-on- exec flag FD_CLOEXEC set */
#define F_GETFD 0x04 /* Get file descriptor flags */
#define F_SETFD 0x08 /* Set file descriptor flags */
#define F_GETFL 0x10 /* Get file status flags and file access modes */
#define F_SETFL 0x20 /* Set file status flags */
#define F_GETLK 0x40 /* Get record locking information */
#define F_SETLK 0x80 /* Set record locking information */
#define F_SETLKW 0x100 /* Set record locking information; wait if blocked */
#define F_GETOWN 0x200 /* Get process or process group ID to receive SIGURG signals */
#define F_SETOWN 0x400 /* Set process or process group ID to receive SIGURG signals */
#define FD_CLOEXEC 0x800 /* Close the file descriptor upon execution of an exec family function */
#define F_RDLCK 0x1000 /* Shared or read lock */
#define F_UNLCK 0x2000 /* Unlock */
#define F_WRLCK 0x4000 /* Exclusive or write lock */

/* Bitwise distinct */
#define O_CLOEXEC 0x01
#define O_CREAT 0x02 /* Create file if it does not exist */
#define O_EXCL 0x04 /* Fail if not a directory */
#define O_NOCTTY 0x08 /* Do not assign controlling terminal */
#define O_NOFOLLOW 0x10 /* Don't follow symlinks */
#define O_TRUNC 0x20 /* Truncate */
#define O_TTY_INIT 0x40 /* 0x00 is valid too */

#define O_ACCMODE 0x80 /* Mask for file acess modes */

/* File status flags */
#define O_APPEND 0x100
#define O_DIRECT 0x200
#define O_DIRECTORY 0x400 /* */
#define O_DSYNC 0x800
#define O_NONBLOCK 0x1000
#define O_RSYNC 0x2000
#define O_SYNC 0x4000
#define O_TMPFILE 0x8000

/* Exec and search may have equal values */
#define O_RDONLY 0x00 /* Read only - should be zero due to historic reasons(?) */
#define O_EXEC 0x10000 /* Execute only */
#define O_SEARCH 0x40000
#define O_WRONLY 0x80000 /* Write only */
#define O_RDWR O_RDONLY | O_WRONLY /* Read and write */

/* Unknown ??? */
#define O_ASYNC 0x00
#define O_LARGEFILE 0x00
#define O_NOATIME 0x00
#define O_PATH 0x00
#define O_NDELAY O_NONBLOCK

int creat(const char *name, mode_t mode);
int open(const char *name, int flags, ...);
int fcntl(int fd, int ctl, ...);

#ifdef __cplusplus
}
#endif

#endif
