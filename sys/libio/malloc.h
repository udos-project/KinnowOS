#ifndef __LIBIO_MALLOC_H__
#define __LIBIO_MALLOC_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <svc.h>

void *malloc(size_t size);
void *calloc(size_t n_memb, size_t size);
void *realloc(void *ptr, size_t size);
void *memalign(size_t align, size_t size);
void free(void *ptr);
#define cfree(p) free(p)
#if _ISOC11_SOURCE
#   define aligned_alloc(a, s) memalign(a, s)
#endif

#ifdef __cplusplus
}
#endif

#endif
