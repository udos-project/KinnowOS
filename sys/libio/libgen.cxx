#include <stddef.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>

STDAPI char *basename(char *path)
{
    char *p;
    p = strpbrk(path, "\\/.");
    while(p != NULL) {
        path = p + 1;
        p = strpbrk(path, "\\/.");
    }
    return (char *)path;
}

static char dntmp[NAME_MAX + 1];
STDAPI char *dirname(char *path)
{
    char *ptr;

    ptr = strchr(path, '/');
    if(ptr == NULL) {
        dntmp[0] = '.';
        dntmp[1] = '\0';
        return dntmp;
    }

    /* Copy until / */
    *ptr = '\0';
    ptr = strncpy(dntmp, ptr, sizeof dntmp);
    return ptr;
}
