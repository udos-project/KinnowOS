#include <locale.h>
#include <string.h>

/* Current locale */
static struct lconv cur_locale;
constinit static struct lconv c_locale = {
    .decimal_point = ".",
    .thousands_sep = ",",
    .grouping = ",",
    .int_curr_symbol = "$",
    .currency_symbol = "$",
    .mon_decimal_point = ".",
    .mon_thousands_sep = ",",
    .mon_grouping = ",",
    .positive_sign = "+",
    .negative_sign = "-",
    .int_frac_digits = 1,
    .frac_digits = 1,
    .p_cs_precedes = 0,
    .p_sep_by_space = 0,
    .n_cs_precedes = 0,
    .n_sep_by_space = 0,
    .p_sign_posn = 0,
    .n_sign_posn = 0,
};

/**
 * @brief Obtains the local locale of the system
 * 
 * @return struct lconv* The pointer to the struct holding the system's locale
 */
STDAPI struct lconv *localeconv(void)
{
    /** @todo A better method for localeconv(), since it only accounts for the current program */
    return &cur_locale;
}

/**
 * @brief Sets the current locale of the program
 * 
 * @param category Category to use
 * @param locale Locale to set
 * @return char* Non-null on success
 */
STDAPI char *setlocale(int category, const char *locale)
{
    const struct lconv *used = nullptr;

    /* Alias */
    if(strcmp(locale, "POSIX") == 0) {
        locale = "C";
    }

    if(strcmp(locale, "C") == 0) {
        used = &c_locale;
    } else {
        /** @todo Other locales */
        return nullptr;
    }

    if(category == LC_ALL) {
        memcpy(&cur_locale, used, sizeof(struct lconv));
        return "LC_ALL";
    }
    /** @todo Other categories beyond LC_ALL */
    return nullptr;
}
