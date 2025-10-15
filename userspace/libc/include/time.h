#ifndef _TIME_H
#define _TIME_H

#include <stddef.h>
#include <stdint.h>

// Time types
typedef long time_t;
typedef long clock_t;

// Clock ticks per second
#define CLOCKS_PER_SEC 1000000L

// Time structure
struct tm {
    int tm_sec;    // seconds after the minute [0-60]
    int tm_min;    // minutes after the hour [0-59]
    int tm_hour;   // hours since midnight [0-23]
    int tm_mday;   // day of the month [1-31]
    int tm_mon;    // months since January [0-11]
    int tm_year;   // years since 1900
    int tm_wday;   // days since Sunday [0-6]
    int tm_yday;   // days since January 1 [0-365]
    int tm_isdst;  // Daylight Saving Time flag
};

// Timespec for nanosecond precision
struct timespec {
    time_t tv_sec;   // seconds
    long   tv_nsec;  // nanoseconds
};

// Time functions
time_t time(time_t *timer);
clock_t clock(void);

// Time conversion
struct tm *localtime(const time_t *timer);
struct tm *gmtime(const time_t *timer);
time_t mktime(struct tm *timeptr);

// Time formatting
char *asctime(const struct tm *timeptr);
char *ctime(const time_t *timer);
size_t strftime(char *str, size_t maxsize, const char *format, const struct tm *timeptr);

// Time difference
double difftime(time_t time1, time_t time0);

// Sleep functions (POSIX extension)
unsigned int sleep(unsigned int seconds);
int usleep(unsigned int usec);
int nanosleep(const struct timespec *req, struct timespec *rem);

#endif // _TIME_H
