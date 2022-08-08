#ifndef __LIBIO_STRING_H__
#define __LIBIO_STRING_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <bits/features.h>
#include <stddef.h>

#if defined(__STDC__) && !defined(__STDC_VERSION__) /* C89 */
void *memcpy(void *__restrict__ dest, const void *__restrict__ src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *__restrict__ s1, const void *__restrict__ s2, size_t n);

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *accept);
char *strpbrk(const char *s, const char *accept);
char *strcpy(char *__restrict__ s1, const char *__restrict__ s2);
char *strncpy(char *__restrict__ s1, const char *__restrict__ s2, size_t n);
char *strcat(char *__restrict__ s1, const char *__restrict__ s2);
char *strncat(char *__restrict__ s1, const char *__restrict__ s2, size_t n);
char *strstr(const char *haystack, const char *needle);
#   if _POSIX_C_SOURCE >= 200809L
char *stpcpy(char *__restrict__ s1, const char *__restrict__ s2);
#   endif
#else /* C99 */
#   include "string.inl"
#endif

char *strdup(const char *s);
char *strndup(const char *s, size_t n);

char *strerror(int err);

void bzero(void *s, size_t n);
void explicit_bzero(void *s, size_t n);

#ifdef __cplusplus
}
#endif

#endif
