#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Init runtime helpers
 * - Logging
 * - Sleeping (spin fallback)
 * - Basic argument parsing
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { RT_INFO=0, RT_WARN=1, RT_ERR=2 } rt_loglvl_t;

void rt_log(rt_loglvl_t lvl, const char* fmt, ...);
void rt_sleep_ms(uint32_t ms);

/* Return 1 if arg like "--key=value" matches key, and fills out into val (ptr into argv string). */
int rt_arg_match(const char* arg, const char* key, const char** out_val);

#ifdef __cplusplus
}
#endif