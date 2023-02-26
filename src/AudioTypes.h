#pragma once

#include "stdint.h"

#ifndef NULL
#define NULL (0)
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define RESTRICT __restrict

typedef bool bool_t;
typedef float float32_t;
typedef const float32_t cfloat32_t;
typedef const int32_t cint32_t;
typedef const uint32_t cuint32_t;
typedef const int16_t cint16_t;
typedef const uint16_t cuint16_t;
