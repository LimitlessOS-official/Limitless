#include <math.h>

// Absolute value for doubles
double fabs(double x) {
    return (x < 0.0) ? -x : x;
}

// Absolute value for floats
float fabsf(float x) {
    return (x < 0.0f) ? -x : x;
}

// Ceiling function
double ceil(double x) {
    long long i = (long long)x;
    if (x > 0.0 && x > (double)i) {
        return (double)(i + 1);
    }
    return (double)i;
}

float ceilf(float x) {
    int i = (int)x;
    if (x > 0.0f && x > (float)i) {
        return (float)(i + 1);
    }
    return (float)i;
}

// Floor function
double floor(double x) {
    long long i = (long long)x;
    if (x < 0.0 && x < (double)i) {
        return (double)(i - 1);
    }
    return (double)i;
}

float floorf(float x) {
    int i = (int)x;
    if (x < 0.0f && x < (float)i) {
        return (float)(i - 1);
    }
    return (float)i;
}

// Round to nearest integer
double round(double x) {
    if (x >= 0.0) {
        return floor(x + 0.5);
    } else {
        return ceil(x - 0.5);
    }
}

float roundf(float x) {
    if (x >= 0.0f) {
        return floorf(x + 0.5f);
    } else {
        return ceilf(x - 0.5f);
    }
}

// Floating-point modulo
double fmod(double x, double y) {
    if (y == 0.0) {
        return NAN;
    }
    double quot = x / y;
    long long iquot = (long long)quot;
    return x - (double)iquot * y;
}

float fmodf(float x, float y) {
    if (y == 0.0f) {
        return NAN;
    }
    float quot = x / y;
    int iquot = (int)quot;
    return x - (float)iquot * y;
}

// Square root using Newton-Raphson method
double sqrt(double x) {
    if (x < 0.0) {
        return NAN;
    }
    if (x == 0.0) {
        return 0.0;
    }
    
    double guess = x;
    double epsilon = 1e-10;
    
    for (int i = 0; i < 50; i++) {
        double new_guess = (guess + x / guess) / 2.0;
        if (fabs(new_guess - guess) < epsilon) {
            return new_guess;
        }
        guess = new_guess;
    }
    
    return guess;
}

float sqrtf(float x) {
    if (x < 0.0f) {
        return NAN;
    }
    if (x == 0.0f) {
        return 0.0f;
    }
    
    float guess = x;
    float epsilon = 1e-6f;
    
    for (int i = 0; i < 30; i++) {
        float new_guess = (guess + x / guess) / 2.0f;
        if (fabsf(new_guess - guess) < epsilon) {
            return new_guess;
        }
        guess = new_guess;
    }
    
    return guess;
}

// Power function using repeated multiplication for integer exponents
double pow(double base, double exponent) {
    if (exponent == 0.0) {
        return 1.0;
    }
    
    if (exponent < 0.0) {
        return 1.0 / pow(base, -exponent);
    }
    
    // For integer exponents
    int exp_int = (int)exponent;
    if ((double)exp_int == exponent) {
        double result = 1.0;
        double current_power = base;
        int n = exp_int;
        
        while (n > 0) {
            if (n & 1) {
                result *= current_power;
            }
            current_power *= current_power;
            n >>= 1;
        }
        return result;
    }
    
    // For fractional exponents, use exp(y * log(x))
    // This is a simplified version
    return exp(exponent * log(base));
}

float powf(float base, float exponent) {
    return (float)pow((double)base, (double)exponent);
}

// Natural logarithm using Taylor series
double log(double x) {
    if (x <= 0.0) {
        return NAN;
    }
    if (x == 1.0) {
        return 0.0;
    }
    
    // Use the approximation: ln(x) ≈ 2 * ((x-1)/(x+1) + 1/3*((x-1)/(x+1))^3 + ...)
    double y = (x - 1.0) / (x + 1.0);
    double y2 = y * y;
    double result = 0.0;
    double term = y;
    
    for (int i = 1; i < 100; i += 2) {
        result += term / (double)i;
        term *= y2;
        if (fabs(term / (double)i) < 1e-10) {
            break;
        }
    }
    
    return 2.0 * result;
}

// Base-10 logarithm
double log10(double x) {
    return log(x) / M_LN10;
}

// Exponential function using Taylor series
double exp(double x) {
    if (x == 0.0) {
        return 1.0;
    }
    
    double result = 1.0;
    double term = 1.0;
    
    for (int i = 1; i < 100; i++) {
        term *= x / (double)i;
        result += term;
        if (fabs(term) < 1e-10) {
            break;
        }
    }
    
    return result;
}
