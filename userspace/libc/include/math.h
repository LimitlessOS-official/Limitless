#ifndef _MATH_H
#define _MATH_H

// Mathematical constants
#define M_E        2.71828182845904523536   // e
#define M_LOG2E    1.44269504088896340736   // log2(e)
#define M_LOG10E   0.43429448190325182765   // log10(e)
#define M_LN2      0.69314718055994530942   // ln(2)
#define M_LN10     2.30258509299404568402   // ln(10)
#define M_PI       3.14159265358979323846   // pi
#define M_PI_2     1.57079632679489661923   // pi/2
#define M_PI_4     0.78539816339744830962   // pi/4
#define M_1_PI     0.31830988618379067154   // 1/pi
#define M_2_PI     0.63661977236758134308   // 2/pi
#define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#define M_SQRT1_2  0.70710678118654752440   // 1/sqrt(2)

// Special values
#define INFINITY   (__builtin_inff())
#define NAN        (__builtin_nanf(""))
#define HUGE_VAL   (__builtin_huge_val())

// Basic functions
double fabs(double x);
double fmod(double x, double y);
double ceil(double x);
double floor(double x);
double round(double x);

// Power and exponential functions
double pow(double x, double y);
double sqrt(double x);
double exp(double x);
double log(double x);
double log10(double x);

// Trigonometric functions
double sin(double x);
double cos(double x);
double tan(double x);
double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double y, double x);

// Hyperbolic functions
double sinh(double x);
double cosh(double x);
double tanh(double x);

// Float versions
float fabsf(float x);
float fmodf(float x, float y);
float ceilf(float x);
float floorf(float x);
float roundf(float x);
float powf(float x, float y);
float sqrtf(float x);
float sinf(float x);
float cosf(float x);
float tanf(float x);

#endif // _MATH_H
