/*
 * Sean James
 *
 * defs.h
 *
 * Common definitions and includes for convenience
 *
 */

#ifndef _DEFS_H
#define _DEFS_H

// Compiling w/ extra errors. Silence GDB variable not used warning
#define TEMPUSE(a) a = a;

// Simple things that should be inlined
#define MIN2(a, b) ((a) < (b) ? (a) : (b))
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(min, max, v) (MIN2(MAX2((min), (v)), (max)))
#define SIGN(x) ((x) > 0 ? 1 : -1)
#define SIGNF(x) ((x) > 0.0f ? 1.0f : -1.0f)
#define INFINITY 10000000.0f // TODO this is stupid

// OS specific settings
#ifdef WINDOWS

#define INLINE _inline
#define PACKED

#else

#define INLINE inline
#define PACKED __attribute__((__packed__))

#endif

// Common includes
#include <vector>
#include <map>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

#define _USE_MATH_DEFINES
#include <math.h>

#endif
