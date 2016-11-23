/**
 * @file rt_defs.h
 *
 * @brief Raytracer global definitions
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __RTDEFS_H
#define __RTDEFS_H

#ifdef WIN32
#ifdef rt_EXPORTS
//#define RT_EXPORT __declspec(dllexport)
#define RT_EXPORT
#else
//#define RT_EXPORT __declspec(dllimport)
#define RT_EXPORT
#endif
#pragma warning(disable: 4251)
#else
#define RT_EXPORT
#endif

#if GPU
#define GLOBAL device
#define THREAD thread
#else
#define GLOBAL
#define THREAD
#endif

#define SIMD 4

#endif
