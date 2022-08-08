#include <string.h>

/**
 * @brief Compare memory s1 and s2 by n characters
 * 
 * @param s1 
 * @param s2 
 * @param n Size of the area to compare
 * @return int Zero if equal, non-zero otherwise
 */
OPT_INLINE int bcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

/**
 * @brief Copies n bytes from s1 into s2
 * 
 * @param s1 Source
 * @param s2 Destination
 * @param n Size of the area
 */
OPT_INLINE void bcopy(const void *s1, void *s2, size_t n)
{
    memmove(s2, s1, n);
    return;
}

/**
 * @brief Zeroes out a piece of the storage
 * Reference: https://man7.org/linux/man-pages/man3/bzero.3.html
 * 
 * @param s Area to zero out
 * @param n Size of the area
 */
OPT_INLINE void bzero(void *s, size_t n)
{
    memset(s, 0, n);
}

/**
 * @brief Same as bzero, but guarantees that the operation won't be
 * optimized away by the compiler if deemed unescesary
 * 
 * @param s Area to zero out
 * @param n Size of the area
 */
OPT_INLINE void explicit_bzero(void *s, size_t n)
{
    memset(s, 0, n);
}

/**
 * @brief Finds the first bit that is set on the specified bitset i
 * and returns the index of said bit
 * 
 * TODO: Is it 1 or zero based?
 * @param i Bitset
 * @return int 
 */
OPT_INLINE int ffs(int i)
{
    int idx = 0;
    if(i == 0) {
        return 0;
    }

    /* Find until bit is set */
    while((i & 1) == 0) {
        i >>= 1;
        idx++;
    }
    return idx;
}

/**
 * @brief Equivalent to strchr(s, c)
 * 
 * @param s 
 * @param c 
 * @return char* 
 */
OPT_INLINE char *index(const char *s, int c)
{
    return strchr(s, c);
}

/**
 * @brief Equivalent to strrchr(s, c)
 * 
 * @param s 
 * @param c 
 * @return char* 
 */
OPT_INLINE char *rindex(const char *s, int c)
{
    return strrchr(s, c);
}
