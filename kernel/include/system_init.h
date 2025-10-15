/*
 * System Initialization Header
 * 
 * Public API for system initialization and health monitoring
 */

#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

// Initialize all kernel subsystems in correct order
int system_init(void);

// Display system information
void system_info(void);

// Run basic self-tests
int system_selftest(void);

// Perform system health check
void system_health_check(void);

#endif // SYSTEM_INIT_H
