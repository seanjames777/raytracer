/**
 * @file defs.h
 *
 * @brief Common definitions and includes
 *
 * @author Sean James
 */

#ifndef _DEFS_H
#define _DEFS_H

#define INLINE inline
#define PACKED __attribute__((__packed__))

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef ASSERT
#define ASSERT(cond) if (!(cond)) { printf("ASSERTION failed on %s:%d\n", __FILE__, __LINE__); __debugbreak(); getchar(); exit(0); }
#endif

#endif
