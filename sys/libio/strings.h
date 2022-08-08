#ifndef __LIBIO_STRINGS_H__
#define __LIBIO_STRINGS_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <bits/features.h>
#include <stddef.h>

#if defined(__STDC__) && !defined(__STDC_VERSION__) /* C89 */
int bcmp(const void *s1, const void *s2, size_t n);
void bcopy(const void *s1, void *s2, size_t n);
void bzero(void *s, size_t n);
void explicit_bzero(void *s, size_t n);
int ffs(int i);
char *index(const char *s, int c);
char *rindex(const char *s, int c);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
#else /* C99 */
#   include "strings.inl"
#endif

#ifdef __cplusplus
}
#endif

#endif
