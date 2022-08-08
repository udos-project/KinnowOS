#ifndef __LIBIO_LDESC_H__
#define __LIBIO_LDESC_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

enum ldesc_license {
    LDESC_LICENSE_INVALID = 0x00, /* - */
    LDESC_LICENSE_PUBLIC_DOMAIN = 0x01, /* Public domain */
    LDESC_LICENSE_CC0 = 0x02, /* Creative commons zero */
    LDESC_LICENSE_UNLICENSE = 0x03, /* Unlicense */
    LDESC_LICENSE_GPLV2 = 0x12, /* GPLv2 */
    LDESC_LICENSE_GPLV3 = 0x13, /* GPLv3 */
    LDESC_LICENSE_MIT = 0x20, /* MIT */
    LDESC_LICENSE_APACHE = 0x30, /* Apache */
    LDESC_LICENSE_BSD = 0x40, /* BSD */
    LDESC_LICENSE_0BSD = 0x41, /* 0-BSD */
    LDESC_LICENSE_CUSTOM = 0x80, /* Custom license */
};

struct ldesc_mod {
    uint8_t en_name[22]; /* English/International name */
    uint8_t jp_name[22]; /* Japanesse name */
    uint8_t license; /* License type */
    void (*start)(void);
    void (*argv_start)(int argc, char **argv);
    void (*exit)(int);
};

#ifdef __cplusplus
}
#endif

#endif
