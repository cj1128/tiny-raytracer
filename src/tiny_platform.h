#ifndef TINY_PLATFORM_H
#define TINY_PLATFORM_H

#define ArrayCount(arr) (sizeof((arr)) / (sizeof((arr)[0])))

#define Assert(expression)                                                     \
  if(!(expression)) {                                                          \
    *(volatile int *)0 = 0;                                                    \
  }

#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32 bool32;
typedef float real32;
typedef double real64;

#endif
