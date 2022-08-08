#ifndef __LIBIO_STDINT_H__
#define __LIBIO_STDINT_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long int64_t;

typedef unsigned long uintptr_t;
typedef signed long intptr_t;
typedef signed long ptrdiff_t;

typedef unsigned long uintmax_t;
typedef signed long intmax_t;

#ifdef __cplusplus
}
#endif

#endif
