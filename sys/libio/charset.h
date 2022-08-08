#ifndef __LIBIO_CHARSET_H__
#define __LIBIO_CHARSET_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <ctype.h>

/* This allows us to basically let the compiler do it's job :) */
#define charset_is_lower_alpha(x) _isloweralpha(x)
#define charset_is_upper_alpha(x) _isupperalpha(x)
#define charset_is_number(x) isdigit(x)
#define charset_is_alpha(x) isalpha(x)
#define charset_is_alnum(x) isalnum(x)
#define charset_to_upper(x) (unsigned char)toupper(x)
#define charset_to_lower(x) (unsigned char)tolower(x)

extern const unsigned char asc2ebc[256];
extern const unsigned char ebc2asc[256];
extern const unsigned char asc2nat[256];
extern const unsigned char nat2ebc[256];

static inline void charset_ebcdic_to_ascii(void *buf, size_t n)
{
    unsigned char *cbuf = (unsigned char *)buf;
    size_t i;

    /* Expand special characters and make everything uppercase */
    for(i = 0; i < n; i++) {
        cbuf[i] = ebc2asc[cbuf[i]];
    }
}

static inline void charset_ascii_to_ebcdic(void *buf, size_t n)
{
    unsigned char *cbuf = (unsigned char *)buf;
    size_t i;

    /* Expand special characters and make everything uppercase */
    for(i = 0; i < n; i++) {
        cbuf[i] = asc2ebc[cbuf[i]];
    }
}

static inline void charset_ascii_to_native(void *buf, size_t n)
{
    unsigned char *cbuf = (unsigned char *)buf;
    size_t i;

    /* Expand special characters and make everything uppercase */
    for(i = 0; i < n; i++) {
        cbuf[i] = asc2nat[cbuf[i] & 0x7F];
        cbuf[i] = charset_to_upper(cbuf[i]);
    }
}

static inline void charset_native_to_ebcdic(void *buf, size_t n)
{
    unsigned char *cbuf = (unsigned char *)buf;
    size_t i;
    
    for(i = 0; i < n; i++) {
        size_t j;
        cbuf[i] = charset_to_upper(cbuf[i]);
        for(j = 0; j < 256; j++) {
            if(nat2ebc[j] == cbuf[i]) {
                break;
            }
        }
        cbuf[i] = nat2ebc[j];
    }
}

#ifdef __cplusplus
}
#endif

#endif
