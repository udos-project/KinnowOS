#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <crt0.h>

char **environ = nullptr;

/** @todo qsort() */
STDAPI void qsort(void *base, size_t n, size_t size, int (*compar)(const void *s1, const void *s2))
{
    while(1);
    return;
}

/// @brief Convert a string into an integer
/// @param s String to interpret
/// @return int The obtained integer
STDAPI int atoi(const char *s)
{
    char is_neg = 0, base = 10;
    int num = 0;

    s += strspn(s, " \t");
    if(*s == '-') {
        is_neg = 1;
        ++s;
    }

    if(*s == '0') {
        ++s;
        if(*s == 'x') {
            base = 16;
            ++s;
        }
    }

    if(base == 10) {
        while(isdigit(*s)) {
            num *= 10;
            num += *s - '0';
            ++s;
        }
    } else if(base == 16) {
        while(isxdigit(*s)) {
            num *= 10;
            num += *s - '0';
            ++s;
        }
    } else {
        dprintf("Base %i not supported yet!\r\n", (int)base);
        assert(0);
    }

    /* Number can't be negative, yet */
    assert(num >= 0);

    if(is_neg) {
        num = -num;
    }
    return num;
}

/**
 * @brief Convert a string into a float
 * 
 * @param s String to interpret
 * @return float The obtained float
 */
STDAPI float atof(const char *s)
{
    char is_neg = 0;
    float num = 0;

    s += strspn(s, " \t");
    if(*s == '-') {
        is_neg = 1;
        ++s;
    }

    /* Interpret digits... */
    while(isdigit(*s)) {
        num *= 10.f;
        num += *s - '0';
        ++s;
    }

    /* Floating point, point, now the numbers after this are
     * going to be divided by 10 */
    if(*s == '.') {
        float div = 10.f, dec = 0.f;

        /* Each new digit is 10 times smaller than the last one
         * so divide by ten times more after each iteration */
        ++s;
        while(isdigit(*s)) {
            int digit = *s - '0';
            if(digit != 0) {
                dec += (float)digit / div;
            }
            div *= 10.f;
            ++s;
        }
        num += dec;
    }

    /* Number can't be negative, yet */
    assert(num >= 0.f);

    if(is_neg) {
        num = -num;
    }
    return num;
}

/// @brief Sets an environment variable on the current program
/// @param name Name of the variable
/// @param value Value to assign
/// @param overwrite Zero means to not change the value if it already exists
/// and non-zero means to update an already existing environment variable if such
/// exists
/// @return int Return code
STDAPI int setenv(const char *name, const char *value, int overwrite)
{
    size_t len;
    int i;

    /* If it's null, has zero length, or has a = character */
    if(name == nullptr || strlen(name) == 0 || strchr(name, '=') != nullptr) {
        errno = -EINVAL;
        return errno;
    }

    if(environ == nullptr) {
        errno = -EINVAL;
        return errno;
    }

    /* Get length (and also additionally, check that the environment variable doesn't exists
     * and if it does do the appropriate action depending on the value of the overwrite parameter) */
    for(i = 0; environ[i] != nullptr; i++) {
        char *p = strchr(environ[i], '=');
        if(p == nullptr) {
            continue;
        }

        /* Compare that names are equal */
        if(!strncmp(name, environ[i], (size_t)((ptrdiff_t)p - (ptrdiff_t)environ[i]))) {
            /* If they are, and overwrite is zero, we don't change anything */
            if(overwrite == 0) {
                return 0;
            }
            /* Otherwise continue searching */
            continue;
        }
    }

    /* Following code deletes/updates environ in some way, so overwrites are to be done */
    assert(overwrite != 0);

    environ = (char **)realloc(environ, (i + 2) * sizeof(char *));
    if(environ == nullptr) {
        errno = -EINVAL;
        return errno;
    }

    /* Has format "name=value" */
    len = strlen(name) + strlen(value) + 2;
    environ[i] = (char *)malloc(len);
    if(environ[i] == nullptr) {
        errno = -EINVAL;
        /** @todo Resize back environ */
        return errno;
    }

    snprintf(environ[i], len, "%s=%s", name, value);
    environ[i + 1] = nullptr;
    return 0;
}

/**
 * @brief Obtains the specified environment variable from environ
 * 
 * @param name The name of the variable
 * @return char* The contents of it
 */
STDAPI char *getenv(const char *name)
{
    int i;
    /* If it's null, has zero length, or has a = character */
    if(name == nullptr || strlen(name) == 0 || strchr(name, '=') != nullptr) {
        errno = -EINVAL;
        return nullptr;
    }

    if(environ == nullptr) {
        errno = -EINVAL;
        return nullptr;
    }

    for(i = 0; environ[i] != nullptr; i++) {
        char *p = strchr(environ[i], '=');
        if(p == nullptr) {
            continue;
        }

        /* Compare that names are equal */
        if(!strncmp(name, environ[i], (size_t)((ptrdiff_t)p - (ptrdiff_t)environ[i]))) {
            /* Skip = character */
            ++p;
            return p;
        }
    }
    return nullptr;
}

/** @todo unsetenv() */
STDAPI int unsetenv(const char *name)
{
    /* If it's null, has zero length, or has a = character */
    if(name == nullptr || strlen(name) == 0 || strchr(name, '=') != nullptr) {
        errno = -EINVAL;
        return errno;
    }
    return 0;
}

/** @todo abort() */
STDAPI void abort(void)
{
    fprintf(stderr, "Abnormal program termination\r\n");
    _exit(EXIT_FAILURE);
}

/**
 * @brief Normal libc exit, simply calls the underlying LIBC exit routine
 * 
 * @param status Status to exit
 */
STDAPI void exit(int status)
{
    _exit(status);
}

/** @todo strtol() */
/** @todo Detect out-of-range numbers and set errno to ERANGE */
STDAPI long strtol(const char *__restrict__ str, char **__restrict__ endptr, int base)
{
    long num = 0;

    if(str == nullptr || endptr == nullptr) {
        errno = -EINVAL;
        return 0;
    }

    /* Skip all spaces */
    while(isspace(*str)) {
        str++;
    }

    while(1);
    return num;
}

/** @todo strtoul() */
/** @todo Detect out-of-range numbers and set errno to ERANGE */
STDAPI unsigned long strtoul(const char *__restrict__ str, char **__restrict__ endptr, int base)
{
    unsigned long num = 0;

    if(str == nullptr || endptr == nullptr) {
        errno = -EINVAL;
        return 0;
    }

    /* Skip all spaces */
    while(isspace(*str)) {
        str++;
    }

    while(1);
    return num;
}

/** @todo strtoll() */
/** @todo Detect out-of-range numbers and set errno to ERANGE */
STDAPI long long strtoll(const char *__restrict__ str, char **__restrict__ endptr, int base)
{
    errno = -ERANGE;
    return 0;
}

/** @todo strtoull() */
/** @todo Detect out-of-range numbers and set errno to ERANGE */
STDAPI unsigned long long strtoull(const char *__restrict__ str, char **__restrict__ endptr, int base)
{
    errno = -ERANGE;
    return 0;
}

STDAPI void libio_llftoa(long double val, char *str)
{
    char numbuf[80];
    size_t i, j = 0;
    long double dec, whole;

    /* Total zero, special case */
    if(val == 0.f) {
        strcpy(&str[0], "0.0");
        return;
    }

    /* Negative sign */
    if(val < 0.f) {
        *(str++) = '-';
        val = -val; /* Make positive so below calculations make sense */
    }

    /* Convert decimals into integers */
    dec = val - truncl(val);
    if(dec < 0.f) {
        dec = -dec;
    }
    while(dec != truncl(dec)) {
        dec *= 10.f;
    }

    /* Decimal point */
    if(dec >= 1.f) {
        while(dec >= 1.f) {
            numbuf[j++] = (char)((int)dec % 10) + '0';
            dec /= 10.f;
        }
    } else {
        numbuf[j++] = '0';
    }

    numbuf[j++] = '.';

    /* Whole-number part of the float */
    whole = truncl(val);
    if(whole >= 1.f) {
        while(whole >= 1.f) {
            numbuf[j++] = (char)((int)whole % 10) + '0';
            whole /= 10.f;
        }
    } else {
        numbuf[j++] = '0';
    }

    for(i = 0; i != j; i++) {
        str[i] = numbuf[(j - 1) - i];
    }
    str[i] = '\0';
}

/* If not inlined seems to be broken */
STDAPI void libio_ulltoa(unsigned long long val, char *str, int base, char is_signed)
{
    char numbuf[24];
    size_t i, j = 0;
    if(val == 0) {
        strcpy(&str[0], "0");
        return;
    }

    if(is_signed && (signed long long)val < 0) {
        *(str++) = '-';
        val = -val;
    }
    
    while(val) {
        uint8_t rem = (uint8_t)(val % (unsigned long long)base);
        numbuf[j] = (rem >= 10) ? rem - 10 + 'A' : rem + '0';
        val /= (unsigned long long)base;
        j++;
    }

    for(i = 0; i != j; i++) {
        str[i] = numbuf[(j - 1) - i];
    }
    str[i] = '\0';
    return;
}
