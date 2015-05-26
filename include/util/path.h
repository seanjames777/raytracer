/**
 * @file util/path.h
 *
 * @brief Path utilities
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __PATH_H
#define __PATH_H

#include <rt_defs.h>
#include <string>

/**
 * @brief Inserts the full path to the directory containing the executable
 * before localPath. This is useful for loading content distributed
 * alongside the executable.
 *
 * @param[in] localPath Path relative to executable directory
 */
RT_EXPORT std::string relToExeDir(std::string localPath);

#endif
