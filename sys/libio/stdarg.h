#ifndef __LIBIO_STDARG_H__
#define __LIBIO_STDARG_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#if 1
#   ifndef __GNUC_VA_LIST
#       define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
#   endif
/* Standard variable argument list */
#   define va_start(v, l) __builtin_va_start(v, l)
#   define va_end(v) __builtin_va_end(v)
#   define va_arg(v, l) __builtin_va_arg(v, l)
/* Strict ANSI-compliance won't allow va_copy */
#   if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L
#       define va_copy(d,s)	__builtin_va_copy(d, s)
#   endif
/* Define it as a compiler extension under the __ namespace */
#   define __va_copy(d, s)	__builtin_va_copy(d, s)
/* Builtin type */
#   if defined __GNUC__
typedef __gnuc_va_list va_list;
#   endif
#else
/* Since PDPCLIB hasn't implemented a proper va_args implementation we have
 * to resort to this */
typedef char *va_list;
#   define va_start(ap, parmN) ap = (char *)&parmN + 4
#   define va_arg(ap, type) *(type *)(ap += sizeof(type), ap - sizeof(type))
#   define va_end(ap) ap = (char *)0
#endif

#ifdef __cplusplus
}
#endif

#endif
