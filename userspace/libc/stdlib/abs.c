#include <stdlib.h>

// Absolute value of an integer
int abs(int n) {
    return (n < 0) ? -n : n;
}

// Absolute value of a long
long labs(long n) {
    return (n < 0) ? -n : n;
}

// Absolute value of a long long
long long llabs(long long n) {
    return (n < 0) ? -n : n;
}
