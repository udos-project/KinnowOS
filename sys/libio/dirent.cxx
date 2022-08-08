#include <dirent.h>
#include <stddef.h>
#include <assert.h>

/** @todo opendir() */
STDAPI DIR *opendir(const char *path)
{
    return nullptr;
}

/** @todo opendir() */
STDAPI int closedir(DIR *dir)
{
    assert(dir != NULL);
    return 0;
}

/** @todo opendir() */
STDAPI struct dirent *readdir(DIR *dir)
{
    assert(dir != NULL);
    return nullptr;
}

/** @todo opendir() */
STDAPI int readdir_r(DIR *__restrict__ dir, struct dirent *__restrict__ entries, struct dirent **__restrict__ entriesp)
{
    assert(dir != NULL);
    return 0;
}

/** @todo opendir() */
STDAPI void rewinddir(DIR *dir)
{
    assert(dir != NULL);
    return;
}

/** @todo opendir() */
STDAPI void seekdir(DIR *dir, long off)
{
    assert(dir != NULL);
    return;
}

/** @todo opendir() */
STDAPI long telldir(DIR *dir)
{
    assert(dir != NULL);
    return 0;
}
