#ifndef __LIBIO_STDLIB_H__
#define __LIBIO_STDLIB_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <bits/features.h>
#include <stddef.h>

#ifndef abs
#   define abs(x) (((x) < 0) ? (-x) : (x))
#endif

/* Declare wide character type */
#ifndef __cplusplus
#   ifndef __WCHAR_T_DEFINED
#       define __WCHAR_T_DEFINED
#       ifndef _WCHAR_T_DEFINED
#           define _WCHAR_T_DEFINED
#       endif
typedef char wchar_t;
#   endif
#endif

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 0

#include <malloc.h>

extern char **environ;

void abort(void) __NORETURN;
void exit(int status) __NORETURN;

void qsort(void *base, size_t n, size_t size, int (*compar)(const void *s1, const void *s2));

int atoi(const char *s);
float atof(const char *s);

int setenv(const char *name, const char *value, int overwrite);
char *getenv(const char *name);
int unsetenv(const char *name);

long strtol(const char *__restrict__ str, char **__restrict__ endptr, int base);
unsigned long strtoul(const char *__restrict__ str, char **__restrict__ endptr, int base);
long long strtoll(const char *__restrict__ str, char **__restrict__ endptr, int base);
unsigned long long strtoull(const char *__restrict__ str, char **__restrict__ endptr, int base);

void libio_ulltoa(unsigned long long val, char *str, int base, char is_signed);
#define itoa(val, str, base) libio_ulltoa((unsigned long long)(val), str, base, 1)
#define litoa(val, str, base) libio_ulltoa((unsigned long long)(val), str, base, 1)
#define ultoa(val, str, base) libio_ulltoa((unsigned long long)(val), str, base, 0)
#define ltoa(val, str, base) libio_ulltoa((unsigned long long)(val), str, base, 1)
#define uptrtoa(val, str, base) libio_ulltoa((unsigned long long)(val), str, base, 0)
#define usizetoa(val, str, base) libio_ulltoa((unsigned long long)(val), str, base, 0)
void libio_llftoa(long double val, char *str);

#ifdef __cplusplus
}
#endif

#endif
