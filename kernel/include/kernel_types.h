#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef unsigned long size_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#endif /* KERNEL_TYPES_H */