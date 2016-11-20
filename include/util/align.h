/**
 * @file util/align.h
 *
 * @brief Aligned memory allocation and pointer alignment utility
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __ALIGN_H
#define __ALIGN_H

#include <stdlib.h>

#define ALIGN_PTR(ptr, align) (((unsigned long long)(ptr) + (align) - 1) & ~((align) - 1))
#define CACHE_LINE 64 // TODO: Might be able to figure it out automatically

/**
 * @brief Allocated aligned memory
 *
 * @param[in] size  Size of memory chunk to allocate
 * @param[in] align Alignment requirement for allocated memory chunk
 *
 * @return Pointer to allocated memory if successful, or null on error
 */
inline void *aligned_alloc(size_t size, size_t align) {
#ifdef WIN32
    return _aligned_malloc(size, align);
#else
    void *ptr = nullptr;

    if (posix_memalign(&ptr, align, size))
        return nullptr;

    return ptr;
#endif
}

#endif
