#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/mman.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

#define local_persist static;
#define global_variable static;
#define ArrayCount(x) sizeof(x)/sizeof(x[0])

#define KB(x) x*1024
#define MB(x) x*1024*1024
#define GB(x) x*1024*1024*1024

#define Assert(expression) \
    if (!(expression)) { \
        fprintf(stderr, "Assertion in file: %s at line %d\n", __FILE__, __LINE__); \
        asm("int3"); \
    }
