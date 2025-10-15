/*
 * Common type definitions for LimitlessOS userspace
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Status/error codes */
typedef int status_t;

#define STATUS_OK           0
#define STATUS_ERROR       -1
#define STATUS_NOMEM       -12
#define STATUS_EXISTS      -17
#define STATUS_INVAL       -22
#define STATUS_NOENT       -2
#define STATUS_BUSY        -16
#define STATUS_TIMEOUT     -110
#define STATUS_NOTSUPP     -95

/* Common data structures */
typedef struct {
    uint64_t size;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint64_t atime;
    uint64_t mtime;
    uint64_t ctime;
} u_stat_t;

/* Color definitions for GUI */
typedef struct {
    uint8_t r, g, b, a;
} color_rgba_t;

/* Rectangle for GUI positioning */
typedef struct {
    int32_t x, y;
    uint32_t width, height;
} rect_t;

/* Point for coordinates */
typedef struct {
    int32_t x, y;
} point_t;

/* Size dimensions */
typedef struct {
    uint32_t width, height;
} gui_size_t;

#ifndef TYPES_H
#define TYPES_H

#endif /* TYPES_H */