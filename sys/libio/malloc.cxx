#include <malloc.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <svc.h>

/**
 * @brief Allocates a piece of storage
 * 
 * @param size Size to allocate
 * @return void* nullptr if allocation fails
 */
STDAPI void *malloc(size_t size)
{
    void *ptr;
    ptr = (void *)io_svc(SVC_GET_STORAGE, (uintptr_t)size, 0, 0);
    if(ptr == nullptr) {
        dprintf("Failed to allocate %u bytes", size);
        errno = -ENOMEM;
    }
    return ptr;
}

/**
 * @brief Allocate and zero-out a piece of storage
 * 
 * @param n_memb Number of members
 * @param size Size of each member
 * @return void* nullptr if allocation fails
 */
STDAPI void *calloc(size_t n_memb, size_t size)
{
    void *ptr;
    ptr = malloc(size * n_memb);
    if(ptr != nullptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

/**
 * @brief Reallocate a piece of storage
 * 
 * @param ptr Previously allocated storage, if nullptr then new storage is allocated
 * @param size The new size of the allocation, if 0 it will automatically free
 * @return void* nullptr if allocation fails or the pointer was freed
 */
STDAPI void *realloc(void *ptr, size_t size)
{
    if(ptr == nullptr) {
        if(size == 0) {
            return nullptr;
        }
        return malloc(size);
    }

    if(size == 0) {
        free(ptr);
        return nullptr;
    }

    ptr = (void *)io_svc(SVC_RESIZE_STORAGE, (uintptr_t)ptr, (uintptr_t)size, 0);
    if(ptr == nullptr && size) {
        errno = -ENOMEM;
    }
    return ptr;
}

/**
 * @brief Releases a piece of storage back to the system
 * 
 * @param ptr Storage to release
 */
STDAPI void free(void *ptr)
{
    io_svc(SVC_DROP_STORAGE, (uintptr_t)ptr, 0, 0);
}
