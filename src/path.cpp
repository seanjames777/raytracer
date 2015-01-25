/**
 * @file path.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <path.h>
#include <sstream>
#include <cassert>
#include <algorithm>

#if defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__linux__)
    #include <linux/limits.h>
    #include <unistd.h>
#elif defined (_WINDOWS)
    #include <Windows.h>
    #define PATH_MAX MAX_PATH
#endif

namespace PathUtil {

std::string prependExecutableDirectory(std::string localPath) {
    char buff[PATH_MAX + 1];

#if defined(__APPLE__)
	uint32_t size = PATH_MAX;
    int stat = _NSGetExecutablePath(buff, &size);
    assert(!stat);
#elif defined (__linux__)
    ssize_t size = readlink("/proc/self/exe", buff, PATH_MAX);
    assert(size >= 0);
#elif defined(_WINDOWS)
	HMODULE module = GetModuleHandle(NULL);
	DWORD size = GetModuleFileNameA(module, buff, PATH_MAX);
    assert(size >= 0);
#endif

	buff[size] = 0;
	std::string exe_path(buff);

#ifdef _WINDOWS
	std::replace(exe_path.begin(), exe_path.end(), '\\', '/');
#endif

    std::string exe_dir = exe_path.substr(0, exe_path.rfind("/"));

    std::stringstream path_str;
    path_str << exe_dir << "/" << localPath;
    std::string path = path_str.str();

    return path;
}

}
