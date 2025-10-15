// LimitlessOS Secure Driver Loader & Sandbox Core
// Implements multi-layered attestation, privilege separation, and real-time monitoring
// Copyright (c) LimitlessOS Project

#ifndef LIMITLESS_DRIVER_LOADER_H
#define LIMITLESS_DRIVER_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include "limitless_driver_api.h"

// Cryptographic signature verification (abstracted for hardware-backed attestation)
int limitless_verify_driver_signature(const struct limitless_driver_manifest *manifest);

// Driver attestation and integrity check
int limitless_attest_driver(const struct limitless_driver_manifest *manifest, const void *driver_binary, size_t binary_size);

// Privilege separation: create isolated execution context for driver
int limitless_create_driver_sandbox(const struct limitless_driver_manifest *manifest);

// Real-time monitoring: kernel hooks for driver resource usage, syscall filtering, and anomaly detection
int limitless_monitor_driver(const char *driver_name);

// Secure loading: atomic load, verify, and initialize driver
int limitless_secure_load_driver(const struct limitless_driver_manifest *manifest, const struct limitless_driver_ops *ops, const void *driver_binary, size_t binary_size);

// Unloading and cleanup
int limitless_secure_unload_driver(const char *driver_name);

// Runtime integrity: periodic re-attestation and rollback on failure
int limitless_periodic_driver_attestation(const char *driver_name);
int limitless_rollback_driver(const char *driver_name);

#endif // LIMITLESS_DRIVER_LOADER_H
