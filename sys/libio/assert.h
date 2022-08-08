#ifndef __LIBIO_ASSERT_H__
#define __LIBIO_ASSERT_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifdef NDEBUG
#   define assert(x) (0)
#else
#   define assert(x) \
    if(!(x)) { \
        printf("assertion " #x " failed!\r\n"); \
        while(1) {}; \
    }
#endif

#define debug_assert(x) assert(x)

#ifdef __cplusplus
}
#endif

#endif
