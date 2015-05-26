/**
 * @file core/raybuffer.h
 *
 * @brief Ray buffer. Accumulates rays and allows for reordering to exploit
 * coherence.
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __RAYBUFFER_H
#define __RAYBUFFER_H

#include <math/ray.h>
#include <rt_defs.h>

#define DEFAULT_RAY_BUFFER_CAPACITY 128 // TODO: Tune

/**
 * @brief Ray buffer
 */
class RayBuffer {
private:

    size_t  _size;      //!< Buffer size
    size_t  _capacity; //!< Buffer capacity
    Ray    *_rays;      //!< Ray array

public:

    /**
     * @brief Constructor
     *
     * @param[in] capacity Initial buffer capacity
     */
    RayBuffer(size_t capacity = DEFAULT_RAY_BUFFER_CAPACITY);

    /**
     * @brief Destructor
     */
    ~RayBuffer();

    /**
     * @brief Add a ray to the buffer
     */
    void enqueue(const Ray & ray);

    /**
     * @brief Remove and return a ray from the buffer
     */
    Ray dequeue();

    /**
     * @brief Whether the buffer is empty
     */
    bool empty() const;

    /**
     * @brief Buffer size
     */
    size_t size() const;

    /**
     * @brief Buffer capacity
     */
    size_t capacity() const;

};

inline RayBuffer::RayBuffer(size_t capacity)
    : _size(0),
      _capacity(capacity),
      _rays(new Ray[capacity])
{
}

inline RayBuffer::~RayBuffer() {
    delete [] _rays;
}

inline void RayBuffer::enqueue(const Ray & ray) {
    if (_size + 1 > _capacity) {
        size_t newcapacity = _capacity << 1;

        Ray *new_rays = new Ray[newcapacity];

        for (size_t i = 0; i < _capacity; i++)
            new_rays[i] = _rays[i];

        delete [] _rays;
        _rays = new_rays;
        _capacity = newcapacity;
    }

    _rays[_size++] = ray;
}

inline Ray RayBuffer::dequeue() {
    return _rays[--_size];
}

inline bool RayBuffer::empty() const {
    return _size == 0;
}

inline size_t RayBuffer::size() const {
    return _size;
}

inline size_t RayBuffer::capacity() const {
    return _capacity;
}

#endif
