#ifndef __LIBIO_DIRENT_H__
#define __LIBIO_DIRENT_H__ 1

#include <bits/features.h>
#include <sys/types.h>

typedef struct _DIR {
    void *handle;
} DIR;

struct dirent {
    ino_t d_ino;
    char d_name[];
};

DIR *opendir(const char *path);
int closedir(DIR *dir);
struct dirent *readdir(DIR *dir);
int readdir_r(DIR *__restrict__ dir, struct dirent *__restrict__ entries, struct dirent **__restrict__ entriesp);
void rewinddir(DIR *dir);
void seekdir(DIR *dir, long off);
long telldir(DIR *dir);

#endif
