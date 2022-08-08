#ifndef __BITS_FEATURES_H__
#define __BITS_FEATURES_H__ 1

#ifndef __cplusplus
#	define __restrict__ restrict
#	define STDAPI extern "C"
#else
#	define __restrict__
#	define STDAPI
#endif

#ifdef __GNUC__
#	define __NORETURN __attribute__((noreturn))
#else
#	define __NORETURN
#endif

#if defined(__STDC__) && !defined(__STDC_VERSION__) /* C89 */
#	define OPT_INLINE STDAPI
#else
#	define OPT_INLINE static inline
#endif

#endif
