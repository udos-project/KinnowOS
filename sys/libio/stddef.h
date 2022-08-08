#ifndef __LIBIO_STDDEF_H__
#define __LIBIO_STDDEF_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#define NULL (void *)0ul

#ifndef __SIZE_T_DEFINED
#	define __SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#ifndef __SSIZE_T_DEFINED
#	define __SSIZE_T_DEFINED
typedef signed long ssize_t;
#endif

#ifndef __cplusplus
#	ifndef __WCHAR_T_DEFINED
#		define __WCHAR_T_DEFINED
typedef short wchar_t;
#	endif
#endif

#ifdef __cplusplus
}
#endif

#endif
