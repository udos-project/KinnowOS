/* limits.h
 * Based on the PDPCLIB limits.h:
 * https://sourceforge.net/p/pdos/gitcode/ci/master/tree/pdpclib/limits.h
 */

#ifndef __LIBIO_LIMITS_H__
#define __LIBIO_LIMITS_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_MAX 24
#define PATH_MAX 255

#define CHAR_BIT 8
#define WORD_BIT 16

#define SCHAR_MIN -128
#define SCHAR_MAX 127
#define UCHAR_MAX 255

#if ('\x80' < 0)
#   define CHAR_MIN SCHAR_MIN
#   define CHAR_MAX SCHAR_MAX
#else
#   define CHAR_MIN 0
#   define CHAR_MAX UCHAR_MAX
#endif

#define MB_LEN_MAX 1

#define SHRT_MIN -SHRT_MAX - 1
#define SHRT_MAX 32767
#define USHRT_MAX ((unsigned short)65535u)

#define INT_MIN -INT_MAX
#define INT_MAX 2147483647
#define UINT_MAX 4294967295u

#define LONG_MIN -LONG_MAX
#define LONG_MAX 2147483647l
#define ULONG_MAX 4294967295ul

#define LLONG_MIN -LONG_MAX
#define LLONG_MAX 9223372036854775807ull
#define ULLONG_MAX 18446744073709551615ull

#ifdef __cplusplus
}
#endif

#endif
