/**
 * @file rt_defs.h
 *
 * @brief Raytracer global definitions
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __RTDEFS_H
#define __RTDEFS_H

#include <cstddef>

#ifdef WIN32
#ifdef rt_EXPORTS
#define RT_EXPORT __declspec(dllexport)
#else
#define RT_EXPORT __declspec(dllimport)
#endif
#pragma warning(disable: 4251)
#else
#define RT_EXPORT
#endif

#endif
