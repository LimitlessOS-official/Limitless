#include "runtime.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void rt_log(rt_loglvl_t lvl, const char* fmt, ...) {
    const char* pfx = "[INFO] ";
    if (lvl == RT_WARN) pfx = "[WARN] ";
    else if (lvl == RT_ERR) pfx = "[ERR ] ";
    fputs(pfx, stdout);
    va_list ap; va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fputc('\n', stdout);
}

void rt_sleep_ms(uint32_t ms) {
    /* Portable-ish sleep using nanosleep if available, else busy loop */
#if defined(_WIN32)
    /* On Windows host builds, Sleep is available; but avoid including Windows headers. */
    clock_t start = clock();
    double ticks = (double)ms * (CLOCKS_PER_SEC / 1000.0);
    while ((clock() - start) < ticks) { /* spin */ }
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    (void)nanosleep(&ts, NULL);
#endif
}

int rt_arg_match(const char* arg, const char* key, const char** out_val) {
    if (!arg || !key) return 0;
    /* "--key=value" */
    const char* a = arg;
    if (a[0] == '-' && a[1] == '-') a += 2;
    size_t i = 0;
    while (key[i] && a[i] && key[i] == a[i]) i++;
    if (key[i] != '\0') return 0;
    if (a[i] != '=') return 0;
    if (out_val) *out_val = a + i + 1;
    return 1;
}