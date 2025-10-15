#include <math.h>

// Sine function using Taylor series
double sin(double x) {
    // Normalize x to [-PI, PI]
    while (x > M_PI) x -= 2.0 * M_PI;
    while (x < -M_PI) x += 2.0 * M_PI;
    
    double result = 0.0;
    double term = x;
    double x2 = x * x;
    
    for (int n = 1; n < 50; n += 2) {
        result += term;
        term *= -x2 / ((double)(n + 1) * (double)(n + 2));
        if (fabs(term) < 1e-10) {
            break;
        }
    }
    
    return result;
}

float sinf(float x) {
    return (float)sin((double)x);
}

// Cosine function using Taylor series
double cos(double x) {
    // Normalize x to [-PI, PI]
    while (x > M_PI) x -= 2.0 * M_PI;
    while (x < -M_PI) x += 2.0 * M_PI;
    
    double result = 0.0;
    double term = 1.0;
    double x2 = x * x;
    
    for (int n = 0; n < 50; n += 2) {
        result += term;
        term *= -x2 / ((double)(n + 1) * (double)(n + 2));
        if (fabs(term) < 1e-10) {
            break;
        }
    }
    
    return result;
}

float cosf(float x) {
    return (float)cos((double)x);
}

// Tangent function
double tan(double x) {
    double c = cos(x);
    if (fabs(c) < 1e-10) {
        return INFINITY;
    }
    return sin(x) / c;
}

float tanf(float x) {
    return (float)tan((double)x);
}

// Arcsine using Newton-Raphson
double asin(double x) {
    if (x < -1.0 || x > 1.0) {
        return NAN;
    }
    if (x == -1.0) {
        return -M_PI_2;
    }
    if (x == 1.0) {
        return M_PI_2;
    }
    
    // Use the identity: arcsin(x) = arctan(x / sqrt(1 - x^2))
    return atan(x / sqrt(1.0 - x * x));
}

// Arccosine
double acos(double x) {
    if (x < -1.0 || x > 1.0) {
        return NAN;
    }
    return M_PI_2 - asin(x);
}

// Arctangent using series approximation
double atan(double x) {
    if (x > 1.0) {
        return M_PI_2 - atan(1.0 / x);
    }
    if (x < -1.0) {
        return -M_PI_2 - atan(1.0 / x);
    }
    
    double result = 0.0;
    double term = x;
    double x2 = x * x;
    
    for (int n = 1; n < 100; n += 2) {
        result += term / (double)n;
        term *= -x2;
        if (fabs(term / (double)n) < 1e-10) {
            break;
        }
    }
    
    return result;
}

// Two-argument arctangent
double atan2(double y, double x) {
    if (x > 0.0) {
        return atan(y / x);
    }
    if (x < 0.0 && y >= 0.0) {
        return atan(y / x) + M_PI;
    }
    if (x < 0.0 && y < 0.0) {
        return atan(y / x) - M_PI;
    }
    if (x == 0.0 && y > 0.0) {
        return M_PI_2;
    }
    if (x == 0.0 && y < 0.0) {
        return -M_PI_2;
    }
    return NAN; // x == 0 and y == 0
}

// Hyperbolic sine
double sinh(double x) {
    return (exp(x) - exp(-x)) / 2.0;
}

// Hyperbolic cosine
double cosh(double x) {
    return (exp(x) + exp(-x)) / 2.0;
}

// Hyperbolic tangent
double tanh(double x) {
    double ep = exp(x);
    double en = exp(-x);
    return (ep - en) / (ep + en);
}
