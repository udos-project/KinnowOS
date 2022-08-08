#include <stdint.h>
#include <assert.h>
#include <math.h>

/**
 * @brief Copies a piece of memory to another non-overlapping memory
 * 
 * @param dest Memory to copy to
 * @param src Memory to copy from
 * @param n Size of copy
 * @return void* Always equal to dest
 */
OPT_INLINE void *memcpy(void *__restrict__ dest, const void *__restrict__ src, size_t n)
{
    const char *c_src = (const char *)src;
    char *c_dest = (char *)dest;

    /* Check for overlapping argument pointers - memcpy wasn't made for overlapped memory */
    assert((size_t)abs((ptrdiff_t)dest - (ptrdiff_t)src) > n);
    while(n) {
        *(c_dest++) = *(c_src++);
        --n;
    }
    return c_dest;
}

/**
 * @brief Copies a piece of memory to another however these can be overlapping
 * 
 * @param dest Memory to copy to
 * @param src Memory to copy from
 * @param n Size of copy
 * @return void* Always equal to dest
 */
OPT_INLINE void *memmove(void *dest, const void *src, size_t n)
{
    const char *c_src = (const char *)src;
    char *c_dest = (char *)dest;

    if((uintptr_t)c_dest < (uintptr_t)c_src) {
        while(n) {
            *(c_dest++) = *(c_src++);
            --n;
        }
    } else {
        c_dest += n;
        c_src += n;
        while(n) {
            *(c_dest--) = *(c_src--);
            --n;
        }
    }
    return c_dest;
}

/**
 * @brief Fill a piece of memory with the specified characters
 * 
 * @param s Memory to fill
 * @param c Character to use
 * @param n Times to copy the character over
 * @return void* Always equal to s
 */
OPT_INLINE void *memset(void *s, int c, size_t n)
{
    char *c_s = (char *)s;
    while(n) {
        *(c_s++) = (char)c;
        --n;
    }
    return s;
}

/**
 * @brief Compare a piece of memory with another
 * 
 * @param s1 
 * @param s2 
 * @param n Size of comparison
 * @return int 0 if equal, nonzero otherwise if there is a difference
 */
OPT_INLINE int memcmp(const void *__restrict__ s1, const void *__restrict__ s2, size_t n)
{
    int diff = 0;
    const char *_s1 = (const char *)s1;
    const char *_s2 = (const char *)s2;

    while(n) {
        diff += *(_s1++) - *(_s2++);
        --n;
    }
    return diff;
}

/**
 * @brief Obtain the length of an string
 * 
 * @param s String
 * @return size_t Obtained length
 */
OPT_INLINE size_t strlen(const char *s)
{
    size_t i = 0;
    while(*s != '\0') {
        ++i;
        ++s;
    }
    return i;
}

/**
 * @brief Obtain the length of an string up to n characters
 * 
 * @param s String
 * @param n Maxmimum length before stopping and returning
 * @return size_t Obtained length
 */
OPT_INLINE size_t strnlen(const char *s, size_t n)
{
    size_t i = 0;
    while(*s != '\0') {
        ++i;
        ++s;
        if(i >= n) {
            return i;
        }
    }
    return i;
}

/**
 * @brief Compare two strings, if they have different lengths
 * then nonzero is returned
 * 
 * @param s1 
 * @param s2 
 * @return int 0 if equal, nonzero otherwise
 */
OPT_INLINE int strcmp(const char *s1, const char *s2)
{
    size_t n = strlen(s1);
    int diff = 0;

    while(n && *s1 != '\0' && *s2 != '\0') {
        diff += *(s1++) - *(s2++);
        --n;
    }

    if(*s1 != *s2) {
        return -1;
    }
    return diff;
}

/**
 * @brief Compare two strings up to n characters, if a terminating
 * NULL is found before n, the comparison stops and returns nonzero
 * 
 * @param s1 
 * @param s2 
 * @param n Max. length of comparison
 * @return int 0 if equal, nonzero otherwise
 */
OPT_INLINE int strncmp(const char *s1, const char *s2, size_t n)
{
    int diff = 0;

    while(n && *s1 != '\0' && *s2 != '\0') {
        diff += *(s1++) - *(s2++);
        --n;
    }

    if(*s1 != *s2 && n > 0) {
        return -1;
    }
    return diff;
}

/**
 * @brief Find a character on a string
 * 
 * @param s String to search in
 * @param c Character that has to be found
 * @return char* NULL if not found, otherwise it's the position where the character was found
 */
OPT_INLINE char *strchr(const char *s, int c)
{
    while(*s != '\0' && *s != (char)c) {
        ++s;
    }

    if(*s == '\0') {
        return (char *)NULL;
    }
    return (char *)s;
}

/**
 * @brief Find a character on a string - and return the last occurrence
 * 
 * @param s String to search in
 * @param c Character that has to be found
 * @return char* NULL if not found, otherwise it's the position where the last instance
 * of this character was found
 */
OPT_INLINE char *strrchr(const char *s, int c)
{
    const char *last = (const char *)NULL;
    while(*s != '\0') {
        if(*s == (char)c) {
            last = s;
        }
        ++s;
    }
    return (char *)last;
}

/**
 * @brief Obtain length of string that consists entirely of characters on the accept
 * 
 * @param s String to span
 * @param accept String of characters for search
 * @return size_t The length of the span before finding characters on accept
 */
OPT_INLINE size_t strspn(const char *s, const char *accept)
{
    size_t spn = 0;

    while(*s != '\0') {
        char is_match = 0;
        size_t i;

        for(i = 0; i < strlen(accept); i++) {
            if(*s != accept[i]) {
                continue;
            }

            is_match = 1;
            ++spn;
            break;
        }

        /* Break once the character is not on accept */
        if(!is_match) {
            return spn;
        }
        ++s;
    }
    return spn;
}

/**
 * @brief Obtain length of string before any character on accept appears
 * 
 * @param s String to span
 * @param accept String of characters that has to be found to end span
 * @return size_t The length of the span before finding characters on accept
 */
OPT_INLINE size_t strcspn(const char *s, const char *accept)
{
    size_t spn = 0;

    while(*s != '\0') {
        char is_match = 0;
        size_t i;

        for(i = 0; i < strlen(accept); i++) {
            if(*s != accept[i]) {
                continue;
            }

            is_match = 1;
            ++spn;
            break;
        }

        /* Break once the character is on accept */
        if(is_match) {
            return spn;
        }
        ++s;
    }
    return spn;
}

/**
 * @brief Find any character on accept in a string
 * 
 * @param s String to search in
 * @param accept The string of characters that has to be found
 * @return char* Position where there is atleast 1 character on accept in the string
 */
OPT_INLINE char *strpbrk(const char *s, const char *accept)
{
    while(*s != '\0') {
        size_t i;
        for(i = 0; i < strlen(accept); i++) {
            if(*s == accept[i]) {
                return (char *)s;
            }
        }
        ++s;
    }
    return (char *)NULL;
}

/**
 * @brief Copy a string to another
 * 
 * @param s1 Destination string
 * @param s2 Source string
 * @return char* Always equal to s1
 */
OPT_INLINE char *strcpy(char *__restrict__ s1, const char *__restrict__ s2)
{
    while(*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *(s1++) = '\0';
    return s1;
}

/**
 * @brief Copy a string to another, up to n characters
 * 
 * @param s1 Destination string
 * @param s2 Source string
 * @param n Maximum allowed characters
 * @return char* Always equal to s1
 */
OPT_INLINE char *strncpy(char *__restrict__ s1, const char *__restrict__ s2, size_t n)
{
    while(n && *s2 != '\0') {
        *(s1++) = *(s2++);
        --n;
    }
    *(s1++) = '\0';
    return s1;
}

/**
 * @brief Concate a string into another
 * 
 * @param s1 String to append into
 * @param s2 String to append from
 * @return char* Always equal to s1
 */
OPT_INLINE char *strcat(char *__restrict__ s1, const char *__restrict__ s2)
{
    while(*s1 != '\0') {
        ++s1;
    }

    while(*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *(s1++) = '\0';
    return s1;
}

/**
 * @brief Concate a string into another, up to n characters
 * 
 * @param s1 String to append into
 * @param s2 String to append from
 * @param n Maximum allowed characters
 * @return char* Always equal to s1
 */
OPT_INLINE char *strncat(char *__restrict__ s1, const char *__restrict__ s2, size_t n)
{
    while(*s1 != '\0') {
        ++s1;
    }

    while(*s2 != '\0' && n) {
        *(s1++) = *(s2++);
        --n;
    }
    *(s1++) = '\0';
    return s1;
}

/**
 * @brief Find a string on a string
 * 
 * @param haystack The string to search in
 * @param needle The string that needs to be found
 * @return const char* NULL if not found, otherwise the position where
 * the string was found
 */
OPT_INLINE char *strstr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    while(*haystack != '\0') {
        if(*haystack == *needle) {
            if(!strncmp(haystack, needle, needle_len)) {
                return (char *)haystack;
            }
        }
        ++haystack;
    }
    return (char *)NULL;
}
