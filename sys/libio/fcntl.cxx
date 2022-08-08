#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>
#include <svc.h>

STDAPI int creat(const char *name, mode_t mode)
{
    return open(name, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

STDAPI int open(const char *name, int flags, ...)
{
    size_t i;
    FILE *fp = nullptr;
    va_list args;
    int tries = 0;

    va_start(args, flags);
    if(strlen(name) > NAME_MAX) {
        errno = -ENAMETOOLONG;
        return errno;
    }

    for(i = 0; i < FOPEN_MAX; i++) {
        if(_files[i].handle == NULL) {
            fp = &_files[i];
            break;
        }
    }

    /* Can't open more files (max reached) */
    if(fp == NULL) {
        va_end(args);
        errno = -ENFILE;
        return errno;
    }

    /** @todo The file creation code can hang when running out of allocation units */
try_again:
    fp->handle = (void *)io_svc(SVC_VFS_OPEN, (uintptr_t)name, (uintptr_t)flags, 0);
    if(fp->handle == NULL) {
        /** @todo Don't create files without the w/truncate flag! */
        if(flags & O_CREAT && tries == 0) {
            /* Couldn't open file, we will try to create it then */
            /* Create a new dataset */
            io_svc(SVC_VFS_ADD_NODE, (uintptr_t)"/TAPE", (uintptr_t)name, 0);
            tries++;
            goto try_again;
        }

        /* If we tried already OR we haven't specified O_CREAT, we will fail */
        va_end(args);
        errno = -EIO;
        return errno;
    }
    va_end(args);
    return (int)i;
}

/** @todo fcntl() */
STDAPI int fcntl(int fd, int ctl, ...)
{
    while(1);
    return 0;
}
