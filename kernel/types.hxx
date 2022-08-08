#ifndef TYPES_HXX
#define TYPES_HXX 1

#if 1
#	ifndef __GNUC_VA_LIST
#		define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
#	endif
/* Standard variable argument list */
#	define va_start(v, l) __builtin_va_start(v, l)
#	define va_end(v) __builtin_va_end(v)
#	define va_arg(v, l) __builtin_va_arg(v, l)
/* Strict ANSI-compliance won't allow va_copy */
#	if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L
#		define va_copy(d,s)	__builtin_va_copy(d, s)
#	endif
/* Define it as a compiler extension under the __ namespace */
#	define __va_copy(d, s)	__builtin_va_copy(d, s)
/* Builtin type */
#	if defined __GNUC__
typedef __gnuc_va_list va_list;
#	endif
#else
/* Since PDPCLIB hasn't implemented a proper va_args implementation we have
 * to resort to this */
typedef char *va_list;
#	define va_start(ap, parmN) ap = (char *)&parmN + 4
#	define va_arg(ap, type) *(type *)(ap += sizeof(type), ap - sizeof(type))
#	define va_end(ap) ap = (char *)0
#endif

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

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef unsigned long uintptr_t;
typedef signed long intptr_t;
typedef signed long ptrdiff_t;

typedef unsigned long uintmax_t;
typedef signed long intmax_t;

#ifdef __GNUC__
#	define UBSAN_FUNC __attribute__((no_sanitize("undefined")))
#	define ALIGNED(x) __attribute__((aligned(x)))
#	define PACKED __attribute__((packed))
#	define SECTION(x) __attribute__((section(x)))
#	define NAKED_FUNC __attribute__((naked)) UBSAN_FUNC
#	define ATTRIB_MALLOC __attribute__((malloc))
#else
#	define UBSAN_FUNC __attribute__((no_sanitize_undefined))
#	error Define your macros here
#endif

#ifdef TARGET_S390
#	if MACHINE >= M_ZARCH
#		define STACK_FRAME_SIZE 160
#	else
#		define STACK_FRAME_SIZE 96
#	endif
#else
#   define STACK_FRAME_SIZE 0 // Used in architectures were the stack frame might be written beyond bounds
#endif

#define STACK_BOTTOM(x) (uintptr_t)&((x)[0])
#define STACK_TOP(x) (uintptr_t)&((x)[(sizeof(x) / sizeof((x)[0])) - STACK_FRAME_SIZE])

static_assert(sizeof(uint8_t) == 1 && sizeof(uint16_t) == 2 && sizeof(uint32_t) == 4 && sizeof(uint64_t) == 8);
static_assert(sizeof(void *) == sizeof(uintptr_t) && sizeof(char *) == sizeof(uintptr_t) && sizeof(int *) == sizeof(uintptr_t)&& sizeof(long *) == sizeof(uintptr_t) && sizeof(ptrdiff_t) == sizeof(uintptr_t));

#endif
