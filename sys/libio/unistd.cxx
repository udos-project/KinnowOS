#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <svc.h>

/**
 * @brief Performs a raw IOCTL on a handle pointer
 * 
 * @param handle The handle to perform the IOCTL on
 * @param cmd Command to execute
 * @param ... Arguments of the command
 * @return int Return code
 */
STDAPI int _rawioctl(void *handle, int cmd, ...)
{
    va_list args;
    int r;

    if(handle == NULL) {
        return -EBADF;
    }

    /* We pass va_list as a pointer */
    va_start(args, cmd);
    r = (int)io_svc(SVC_VFS_IOCTL, (uintptr_t)handle, (uintptr_t)cmd, (uintptr_t)&args);
    va_end(args);
    return r;
}

STDAPI int getpagesize(void)
{
    return 4096;
}

STDAPI ssize_t write(int fd, const void *buf, size_t size)
{
    ssize_t r;
    if(fd < 0 || fd > FOPEN_MAX) {
        return -EBADF;
    }
    r = (ssize_t)fwrite(buf, size, 1, &_files[fd]);
    return r;
}

STDAPI ssize_t read(int fd, void *buf, size_t size)
{
    ssize_t r;
    if(fd < 0 || fd > FOPEN_MAX) {
        return -EBADF;
    }
    r = (ssize_t)fread(buf, size, 1, &_files[fd]);
    return r;
}

STDAPI int seek(int fd, long offset, int whence)
{
    if(fd < 0 || fd > FOPEN_MAX) {
        return -EBADF;
    }

    if(whence != SEEK_SET || whence != SEEK_END || whence != SEEK_CUR) {
        return -EINVAL;
    }
    return fseek(&_files[fd], offset, whence);
}

STDAPI int ioctl(int fd, int cmd, ...)
{
    va_list args;
    int r;

    if(fd < 0 || fd > FOPEN_MAX) {
        return -EBADF;
    }

    /* We pass va_list as a pointer */
    va_start(args, cmd);
    r = (int)io_svc(SVC_VFS_IOCTL, (uintptr_t)_files[fd].handle, (uintptr_t)cmd, (uintptr_t)&args);
    va_end(args);
    return r;
}

STDAPI int flush(int fd) {
    if(fd < 0 || fd > FOPEN_MAX) {
        return -EBADF;
    }
    return fflush(&_files[fd]);
}

STDAPI void close(int fd) {
    if(fd < 0 || fd > FOPEN_MAX) {
        return;
    }
    fclose(&_files[fd]);
}

STDAPI off_t lseek(int fd, off_t offset, int whence)
{
    return (off_t)seek(fd, offset, whence);
}

/* Reference:
 * https://man7.org/linux/man-pages/man2/dup.2.html */

/**
 * @brief Duplicates a file descriptor
 * 
 * @param fd The fd to duplicate
 * @return int EBADF if the fd is out of range or not open
 */
STDAPI int dup(int fd)
{
    size_t i;
    /* Check if fd is invalid */
    if(fd < 0 || fd > FOPEN_MAX || _files[fd].handle == NULL) {
        errno = -EBADF;
        return errno;
    }

    /* Find a suitable fd */
    for(i = 0; i < FOPEN_MAX; i++) {
        if(_files[i].handle == NULL) {
            /** @todo Does dup also make the newfd inherit flags?
             * can these diverge? */
            _files[i].flags = _files[fd].flags;
            _files[i].handle = _files[fd].handle;
            return (int)i;
        }
    }

    errno = -EBADF;
    return errno;
}

STDAPI int dup2(int fd, int newfd)
{
    /* Check if fd is invalid or newfd is invalid */
    if(fd < 0 || fd > FOPEN_MAX || newfd < 0 || newfd > FOPEN_MAX || _files[fd].handle == NULL) {
        errno = -EBADF;
        return errno;
    }

    if(fd == newfd) {
        return newfd;
    }

    /* Close the new fd before duping */
    if(_files[newfd].handle != NULL) {
        fclose(&_files[newfd]);
    }

    _files[newfd].flags = _files[fd].flags;
    _files[newfd].handle = _files[fd].handle;
    return newfd;
}

STDAPI int dup3(int fd, int newfd, int flags)
{
    /** @todo Return -EINVAL on invalid flags */
    if(fd < 0 || fd > FOPEN_MAX || newfd < 0 || newfd > FOPEN_MAX || _files[fd].handle == NULL) {
        errno = -EBADF;
        return errno;
    }

    if(fd == newfd) {
        errno = -EINVAL;
        return errno;
    }

    /* Close the new fd before duping */
    if(_files[newfd].handle != NULL) {
        fclose(&_files[newfd]);
    }

    _files[newfd].flags = flags;
    _files[newfd].handle = _files[fd].handle;
    return newfd;
}

STDAPI int access(const char *pathname, int mode)
{
    int fd;
    fd = open(pathname, mode);
    if(fd < 0) {
        /* errno is already set by open */
        return errno;
    }
    close(fd);
    return 0;
}

STDAPI pid_t fork(void)
{
    assert(0);
    return (pid_t)-1;
}

STDAPI int execl(const char *path, const char *arg0, ...)
{
    assert(0);
    return 0;
}

STDAPI int execv(const char *path, char *const argv[])
{
    assert(0);
    return 0;
}

STDAPI int execle(const char *path, const char *arg0, ...)
{
    assert(0);
    return 0;
}

STDAPI int execve(const char *path, char *const argv[], char *const envp[])
{
    assert(0);
    return 0;
}

STDAPI int execlp(const char *file, const char *arg0, ...)
{
    assert(0);
    return 0;
}

STDAPI int execvp(const char *file, char *const argv[])
{
    assert(0);
    return 0;
}
