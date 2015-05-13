#ifndef __RT_DEFS_H
#define __RT_DEFS_H

#ifdef rt_EXPORTS
#define RT_EXPORT __declspec(dllexport)
#else
#define RT_EXPORT __declspec(dllimport)
#endif

#pragma warning(disable: 4251)

#endif
