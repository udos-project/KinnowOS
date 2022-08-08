#ifndef __LIBIO_UNISTD_H__
#define __LIBIO_UNISTD_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Reference for UNISTD.H:
 * https://pubs.opengroup.org/onlinepubs/9699919799.2016edition/basedefs/unistd.h.html
 */

/* Internals */
int _rawioctl(void *handle, int cmd, ...);

int getpagesize(void);

ssize_t write(int fd, const void *buf, size_t size);
ssize_t read(int fd, void *buf, size_t size);
int seek(int fd, long offset, int whence);
int flush(int fd);
int ioctl(int fd, int cmd, ...);
void close(int fd);

off_t lseek(int fd, off_t offset, int whence);

int dup(int fd);
int dup2(int fd, int newfd);
int dup3(int fd, int newfd, int flags);

int access(const char *pathname, int mode);

pid_t fork(void);

int execl(const char *path, const char *arg0, ...);
int execv(const char *path, char *const argv[]);
int execle(const char *path, const char *arg0, ...);
int execve(const char *path, char *const argv[], char *const envp[]);
int execlp(const char *file, const char *arg0, ...);
int execvp(const char *file, char *const argv[]);

#ifdef __cplusplus
}
#endif

#endif
