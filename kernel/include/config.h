#pragma once
/* Global build-time configuration toggles. Adjust as needed for production builds. */

/* Enable filesystem test suites (ext4 extents, etc.) during early bootstrap after mounting. */
#define CONFIG_FS_TESTS 1

/* Default kernel log level (can be overridden at runtime). */
#ifndef CONFIG_KLOG_DEFAULT_LEVEL
#define CONFIG_KLOG_DEFAULT_LEVEL KLOG_L_INFO
#endif

/* Maximum compiled-in verbosity (messages below this are compiled out). */
#ifndef CONFIG_KLOG_COMPILED_MIN_LEVEL
#define CONFIG_KLOG_COMPILED_MIN_LEVEL KLOG_L_DEBUG
#endif
