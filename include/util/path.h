/**
 * @file path.h
 *
 * @brief Path utilities
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __PATH_H
#define __PATH_H

#include <string>
#include <rt_defs.h>

namespace util {

/**
 * @brief Inserts the full path to the directory containing the executable
 * before a 'local' path. This is useful for loading content distributed
 * alongside the executable.
 */
RT_EXPORT std::string prependExecutableDirectory(std::string localPath);

}

#endif
