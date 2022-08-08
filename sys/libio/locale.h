/* locale.h
 * 
 * Based on the locale.h from PDPCLIB:
 * https://sourceforge.net/p/pdos/gitcode/ci/master/tree/pdpclib/locale.h */

#ifndef __LIBIO_LOCALE_H__
#define __LIBIO_LOCALE_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *int_curr_symbol;
    char *currency_symbol;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char int_frac_digits;
    char frac_digits;
    char p_cs_precedes;
    char p_sep_by_space;
    char n_cs_precedes;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
};

#define LC_ALL 1
#define LC_COLLATE 2
#define LC_CTYPE 3
#define LC_MONETARY 4
#define LC_NUMERIC 5
#define LC_TIME 6

#define LC_MESSAGES 7

struct lconv *localeconv(void);
char *setlocale(int category, const char *locale);

#ifdef __cplusplus
}
#endif

#endif
