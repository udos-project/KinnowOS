/* math.h
 *
 * Normal LIBC MATH.H header file, reference:
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/math.h.html
 */

#ifndef __LIBIO_MATH_H__
#define __LIBIO_MATH_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#define NAN (0.f / 0.f)

/* Iterations to perform to the taylor series */
#define _MATH_TAYLOR_ITER 10

/* Eulers constant */
#define M_E 2.71f

/* Pi stuff */
#define M_PI 3.1416f
#define M_PI_2 (M_PI / 2.f)
#define M_PI_4 (M_PI / 4.f)

#define M_1_PI (1.f / M_PI)
#define M_2_PI (2.f / M_PI)

#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2

/* UDOS is paranoid about anything a program does but we asked it to be nice
 * with our program, see crt0.c initialization code for mathematics error handling */
#define math_errhandling MATH_ERRNO | MATH_ERREXCEPT

#define max(x, y) ((x > y) ? (x) : (y))
#define min(x, y) ((x < y) ? (x) : (y))

/* Non-standard */
#define minmax(x, min, max) max(min(x, max), min)
#define clamp(x, min, max) minmax(x, min, max)
#ifndef abs
#   define abs(x) (((x) < 0) ? -(x) : (x))
#endif

typedef float float_t;
typedef double double_t;

float logrf(float a, float b);
double logr(double a, double b);
long double logrl(long double a, long double b);

float logf(float a);
double log(double a);
long double logl(long double a);

float log2f(float x);
double log2(double x);
long double log2l(long double x);

float log10f(float x);
double log10(double x);
long double log10l(long double x);

float powf(float a, float b);
double pow(double a, double b);
long double powl(long double a, long double b);

float factf(float x);
double fact(double x);
long double factl(long double x);

float truncf(float x);
double trunc(double x);
long double truncl(long double x);

float fmodf(float a, float b);
double fmod(double a, double b);
long double fmodl(long double a, long double b);

float sqrtf(float x);
double sqrt(double x);
long double sqrtl(long double x);

float expf(float x);
double exp(double x);
long double expl(long double x);

float exp2f(float x);
double exp2(double x);
long double exp2l(long double x);

/* Trigonometry */
float sinf(float x);
double sin(double x);
long double sinl(long double x);

float cosf(float x);
double cos(double x);
long double cosl(long double x);

float tanf(float x);
double tan(double x);
long double tanl(long double x);

/* Hyperbolic trigonometric functions */
float sinhf(float x);
double sinh(double x);
long double sinhl(long double x);

float coshf(float x);
double cosh(double x);
long double coshl(long double x);

float tanhf(float x);
double tanh(double x);
long double tanhl(long double x);

/* Inverse trigonometric functions */
/* Trigonometry */
float arcsinf(float x);
double arcsin(double x);
long double arcsinl(long double x);

float arccosf(float x);
double arccos(double x);
long double arccosl(long double x);

float arctanf(float x);
double arctan(double x);
long double arctanl(long double x);

/* Extensions */
float gcdf(float a, float b);
double gcd(double a, double b);
long double gcdl(long double a, long double b);

float lcmf(float a, float b);
double lcm(double a, double b);
long double lcml(long double a, long double b);

/**
 * @brief Find the derivate of function f
 * 
 * @param x Input fo f
 * @param h The step to take so that h=(f(x-step))
 * @param f Function f()
 * @return type The derivate
 */
#define DERIVATE(x, h, f) (f(x + h) - f(x)) / h

#ifdef __cplusplus
}
#endif

#endif
