/* math.c
 *
 * Implements all the mathematical tomfuckery, reference:
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/math.h.html
 */
#include <math.h>
#include <bits/features.h>

/**
 * @brief Obtains the logarithm of A with base Y
 * 
 * @param a Real number to obtain logarithm from
 * @param b Base
 * @return type Result of the operation
 */
#define MATH_DECL_LOGR(pf, type) \
STDAPI type logr ##pf (type a, type b) \
{ \
    return (a > b - (type)1.f) ? (type)1.f + logr ##pf(a / b, b) : (type)0.f; \
}

MATH_DECL_LOGR(f, float)
MATH_DECL_LOGR(/* ... */ , double)
MATH_DECL_LOGR(l, long double)

/**
 * @brief Obtains the logarithm of A
 * 
 * @param a Real number to obtain logarithm from
 * @return type Result of the operation
 */
#define MATH_DECL_LOG(pf, type) \
STDAPI type log ##pf (type a) \
{ \
    return logr ##pf(a, (type)1.f); \
}

MATH_DECL_LOG(f, float)
MATH_DECL_LOG(/* ... */ , double)
MATH_DECL_LOG(l, long double)

/* Families of logarithm functions that MUST be implemented go below */
#define MATH_DECL_LOG2(pf, type) \
STDAPI type log2 ##pf (type a) \
{ \
    return logr ##pf(a, (type)2.f); \
}

MATH_DECL_LOG2(f, float)
MATH_DECL_LOG2(/* ... */ , double)
MATH_DECL_LOG2(l, long double)

#define MATH_DECL_LOG10(pf, type) \
STDAPI type log10 ##pf (type a) \
{ \
    return logr ##pf(a, (type)10.f); \
}

MATH_DECL_LOG10(f, float)
MATH_DECL_LOG10(/* ... */ , double)
MATH_DECL_LOG10(l, long double)

/**
 * @brief Obtains the power of x elevated to the y
 * 
 * @param a Real number to obtain power from
 * @param b Power to elevate into
 * @return type Result of the operation
 */
#define MATH_DECL_POW(pf, type) \
STDAPI type pow ##pf (type a, type b) \
{ \
    type r = 1.f; \
    if(b < (type)0.f) { \
        while(b <= (type)1.f) { \
            r = (type)1.f / a; \
            b += (type)1.f; \
        } \
    } else { \
        while(b >= (type)1.f) { \
            r *= a; \
            b -= (type)1.f; \
        } \
    } \
    return r; \
}

MATH_DECL_POW(f, float)
MATH_DECL_POW(/* ... */ , double)
MATH_DECL_POW(l, long double)

/**
 * @brief Obtains the factorial of x
 * 
 * @param x Real number to obtain factorial from
 * @return type Result of the operation
 */
#define MATH_DECL_FACT(pf, type) \
STDAPI type fact ##pf (type x) \
{ \
    type b = 1; \
    while(x) { \
        b *= x; \
        x -= 1; \
    } \
    return b; \
}

MATH_DECL_FACT(f, float)
MATH_DECL_FACT(/* ... */ , double)
MATH_DECL_FACT(l, long double)

/**
 * @brief Truncates a float
 * Reference: https://pubs.opengroup.org/onlinepubs/9699919799/functions/trunc.html
 * 
 * @param a Real number
 * @return type Result of the operation
 */
#define MATH_DECL_TRUNC(pf, type) \
STDAPI type trunc ##pf (type x) \
{ \
    return (type)((signed long long int)x); \
}

MATH_DECL_TRUNC(f, float)
MATH_DECL_TRUNC(/* ... */ , double)
MATH_DECL_TRUNC(l, long double)

/**
 * @brief Modulous implementation for floats
 * 
 * @param a Real number
 * @param b Real divisor to obtain the demainder from
 * @return type Result of the operation
 */
#define MATH_DECL_FMOD(pf, type) \
STDAPI type fmod ##pf (type a, type b) \
{ \
    return a - (trunc ##pf(a / b) * b); \
}

MATH_DECL_FMOD(f, float)
MATH_DECL_FMOD(/* ... */ , double)
MATH_DECL_FMOD(l, long double)

/**
 * @brief Obtains the square root of x
 * 
 * @param x The real number to obtain the square root from
 * @return type Square root
 */
#define MATH_DECL_SQRT(pf, type) \
STDAPI type sqrt ##pf (type x) \
{ \
    int digits = 0, i; \
    type y = x; \
    while(y > (type)0.f) { \
        y /= (type)10.f; \
        digits++; \
    } \
    y = (type)digits * (type)100.f; \
    for(i = 0; i < 100; i++) { \
        y = ((type)1.f / (type)2.f) * (y + x / y); \
    } \
    return y; \
}

MATH_DECL_SQRT(f, float)
MATH_DECL_SQRT(/* ... */ , double)
MATH_DECL_SQRT(l, long double)

/**
 * @brief Exponentiates a number e^x
 * 
 * @param a Number to exponentiate
 * @return type Result of the operation
 */
#define MATH_DECL_EXP(pf, type) \
STDAPI type exp ##pf (type a) \
{ \
    return pow ##pf((type)M_E, a); \
}

MATH_DECL_EXP(f, float)
MATH_DECL_EXP(/* ... */ , double)
MATH_DECL_EXP(l, long double)

/**
 * @brief Exponentiates a number 2^x
 * 
 * @param a Number to exponentiate
 * @return type Result of the operation
 */
#define MATH_DECL_EXP2(pf, type) \
STDAPI type exp2 ##pf (type a) \
{ \
    return pow ##pf((type)2, a); \
}

MATH_DECL_EXP2(f, float)
MATH_DECL_EXP2(/* ... */ , double)
MATH_DECL_EXP2(l, long double)

/**
 * @brief Obtain the sine of x
 * 
 * @param x Radians to get the sine of
 * @return type Sine of x
 */
#define MATH_DECL_SIN(pf, type) \
STDAPI type sin ##pf (type x) \
{ \
    type a = (type)0.f;\
    int n; \
    for(n = 0; n < _MATH_TAYLOR_ITER; n++) { \
        type num = pow ##pf ((type)-1.f, (type)n) / fact ##pf((type)2.f * (type)n + (type)1.f); \
        a += num * pow ##pf (x, (type)2.f * (type)n - (type)1.f); \
    } \
    return a; \
}

MATH_DECL_SIN(f, float)
MATH_DECL_SIN(/* ... */ , double)
MATH_DECL_SIN(l, long double)

/**
 * @brief Obtain the cosine of x
 * 
 * @param x Radians to get cosine of
 * @return type Cosine of x
 */
#define MATH_DECL_COS(pf, type) \
STDAPI type cos ##pf (type x) \
{ \
    return sin ##pf ((type)M_PI_2 - x); \
}

MATH_DECL_COS(f, float)
MATH_DECL_COS(/* ... */ , double)
MATH_DECL_COS(l, long double)

/**
 * @brief Obtain the tangent of x
 * 
 * @param x Radians to get tangent of
 * @return type Tangent of x
 */
#define MATH_DECL_TAN(pf, type) \
STDAPI type tan ##pf (type x) \
{ \
    return sin ##pf (x) / cos ##pf (x); \
}

MATH_DECL_TAN(f, float)
MATH_DECL_TAN(/* ... */ , double)
MATH_DECL_TAN(l, long double)

/**
 * @brief Obtain the hyperbolic sine of x
 * 
 * @param x Radians to get the sine of
 * @return type Sine of x
 */
#define MATH_DECL_SINH(pf, type) \
STDAPI type sinh ##pf (type x) \
{ \
    return (exp ##pf(x) - exp ##pf(-x)) / 2; \
}

MATH_DECL_SINH(f, float)
MATH_DECL_SINH(/* ... */ , double)
MATH_DECL_SINH(l, long double)

/**
 * @brief Obtain the hyperbolic cosine of x
 * 
 * @param x Radians to get cosine of
 * @return type Cosine of x
 */
#define MATH_DECL_COSH(pf, type) \
STDAPI type cosh ##pf (type x) \
{ \
    return sinh ##pf ((type)M_PI_2 - x); \
}

MATH_DECL_COSH(f, float)
MATH_DECL_COSH(/* ... */ , double)
MATH_DECL_COSH(l, long double)

/**
 * @brief Obtain the tangent of x
 * 
 * @param x Radians to get tangent of
 * @return type Tangent of x
 */
#define MATH_DECL_TANH(pf, type) \
STDAPI type tanh ##pf (type x) \
{ \
    return sinh ##pf (x) / sinh ##pf ((type)M_PI_2 - x); \
}

MATH_DECL_TANH(f, float)
MATH_DECL_TANH(/* ... */ , double)
MATH_DECL_TANH(l, long double)

/**
 * @brief Obtain the arcsine of x
 * 
 * @param x Radians to get the arcsine of
 * @return type Sine of x
 */
#define MATH_DECL_ARCSIN(pf, type) \
STDAPI type arcsin ##pf (type x) \
{ \
    type a = (type)0.f; \
    int n; \
    for(n = 0; n < _MATH_TAYLOR_ITER; n++) { \
        type num, den; \
        num = fact ##pf((type)(2 * (type)n)); \
        den = pow ##pf(4, (type)n); \
        den *= pow ##pf(fact ##pf((type)n), 2); \
        den *= 2 * (type)n + 1; \
        a += (num / den) * pow ##pf(x, 2 * (type)n + 1); \
    } \
    return a;\
}

MATH_DECL_ARCSIN(f, float)
MATH_DECL_ARCSIN(/* ... */ , double)
MATH_DECL_ARCSIN(l, long double)

/**
 * @brief Obtain the arccosine of x
 * 
 * @param x Radians to get the arccosine of
 * @return type Arccosine of x
 */
#define MATH_DECL_ARCCOS(pf, type) \
STDAPI type arccos ##pf (type x) \
{ \
    return (type)M_2_PI - arcsin ##pf(x);\
}

MATH_DECL_ARCCOS(f, float)
MATH_DECL_ARCCOS(/* ... */ , double)
MATH_DECL_ARCCOS(l, long double)

/**
 * @brief Obtain the arctangent of x
 * 
 * @param x Radians to get the arctangent of
 * @return type Arctangent of x
 */
#define MATH_DECL_ARCTAN(pf, type) \
STDAPI type arctan ##pf (type x) \
{ \
    type a = (type)0.f; \
    int n; \
    for(n = 0; n < _MATH_TAYLOR_ITER; n++) { \
        type num, den; \
        num = pow ##pf(-1, (type)n); \
        den = 2 * (type)n + 1; \
        a += (num / den) * pow ##pf(x, 2 * (type)n + 1); \
    } \
    return a;\
}

MATH_DECL_ARCTAN(f, float)
MATH_DECL_ARCTAN(/* ... */ , double)
MATH_DECL_ARCTAN(l, long double)

/**
 * @brief Find the greatest common divisor using the Eucledian algorithm, Source:
 * https://en.wikipedia.org/wiki/Euclidean_algorithm
 * 
 * @param a 
 * @param b 
 * @return type The greatest common divisor
 */
#define MATH_DECL_GCD(pf, type) \
STDAPI type gcd ##pf (type a, type b) \
{ \
    if(a == (type)0.f) { \
        return b; \
    } \
    return gcd ##pf (fmod ##pf (b, a), b); \
}

MATH_DECL_GCD(f, float)
MATH_DECL_GCD(/* ... */ , double)
MATH_DECL_GCD(l, long double)

/**
 * @brief Find the least common multiple using the |ab|/gcd(a,b) method, this is by using
 * the Eucledian Algorithm to find the LCM
 * 
 * @param a 
 * @param b 
 * @return type The least common multiple
 */
#define MATH_DECL_LCM(pf, type) \
STDAPI type lcm ##pf (type a, type b) \
{ \
    return (a * b) / gcd ##pf (a, b); \
}

MATH_DECL_LCM(f, float)
MATH_DECL_LCM(/* ... */ , double)
MATH_DECL_LCM(l, long double)
