#include <strings.h>
#include <string.h>
#include <ctype.h>

#if defined(__STDC__) && !defined(__STDC_VERSION__) /* C89 */
#   include "strings.inl"
#endif

/** @todo strcasecmp() */
STDAPI int strcasecmp(const char *s1, const char *s2)
{
    while(*s1 != '\0' && *s2 != '\0') {
        if(toupper(*(s1++)) != toupper(*(s2++))) {
            return 1;
        }
    }

    /* Length difference */
    if(*s1 != '\0' || *s2 != '\0') {
        return 1;
    }
    return 0;
}

/** @todo strncasecmp() */
STDAPI int strncasecmp(const char *s1, const char *s2, size_t n)
{
    size_t i;
    while(*s1 != '\0' && *s2 != '\0' && n) {
        if(toupper(*(s1++)) != toupper(*(s2++))) {
            return 1;
        }
        --n;
    }

    /* Length difference */
    if(*s1 != '\0' || *s2 != '\0') {
        return 1;
    }
    return 0;
}
