#pragma once
#include "kernel.h"
#include "config.h"

typedef enum { KLOG_L_DEBUG=0, KLOG_L_INFO=1, KLOG_L_WARN=2, KLOG_L_ERROR=3 } klog_level_t;

void klog_init(void);
void klog_set_level(klog_level_t lvl);
void klog_vprintf(klog_level_t lvl, const char* tag, const char* fmt, __builtin_va_list ap);
void klog_printf(klog_level_t lvl, const char* tag, const char* fmt, ...);
int  klog_read(char* buf, u32 maxlen); /* non-blocking read oldest chunk */

#if CONFIG_KLOG_COMPILED_MIN_LEVEL <= KLOG_L_DEBUG
#define KLOG_DEBUG(tag, fmt, ...) klog_printf(KLOG_L_DEBUG, tag, fmt, ##__VA_ARGS__)
#else
#define KLOG_DEBUG(tag, fmt, ...) do{}while(0)
#endif
#if CONFIG_KLOG_COMPILED_MIN_LEVEL <= KLOG_L_INFO
#define KLOG_INFO(tag, fmt, ...)  klog_printf(KLOG_L_INFO,  tag, fmt, ##__VA_ARGS__)
#else
#define KLOG_INFO(tag, fmt, ...)  do{}while(0)
#endif
#if CONFIG_KLOG_COMPILED_MIN_LEVEL <= KLOG_L_WARN
#define KLOG_WARN(tag, fmt, ...)  klog_printf(KLOG_L_WARN,  tag, fmt, ##__VA_ARGS__)
#else
#define KLOG_WARN(tag, fmt, ...)  do{}while(0)
#endif
#define KLOG_ERROR(tag, fmt, ...) klog_printf(KLOG_L_ERROR, tag, fmt, ##__VA_ARGS__)
