/**
 * @file util/queue.h
 *
 * @brief Queue/FIFO
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __QUEUE_H
#define __QUEUE_H

#include <cassert>
#include <stdlib.h>
#include <string.h>

// Default initial queue capacity
#define DEFAULT_INIT_QUEUE_CAPACITY 16

namespace util {

/**
 * @brief Generic circular queue/FIFO. Dynamically resizes as needed.
 *
 * @tparam T Element type
 */
template<typename T>
class queue {
private:

    T            *_elems;    // Element buffer
    unsigned int  _head;     // First element
    unsigned int  _tail;     // One past end of queue, unless it is empty, wrapped around capacity
    unsigned int  _size;     // Current number of elements. TODO: can be computed from _head/_tail
    unsigned int  _capacity; // Maximum number of elements

    // Notes:
    //   - We use malloc/free for the elements here, because we overwrite the allocated
    //     space with actual values.
    //   - Initially, capacity is used to indicate the desired capacity, but _elems is null.
    //   - Insertion is allowed to fail, signalling out-of-memory.

    // TODO:
    //   - We actually need to call constructors and destructors
    //   - Need to respect alignment
    //   - reserve() and bounds check free enqueue
    //   - inline functions
    //   - avoid copy on insert

public:

    /**
     * @brief Constructor
     *
     * @param capacity Desired initial capacity of the queue. Must be greater than 0.
     */
    queue(unsigned int capacity = DEFAULT_INIT_QUEUE_CAPACITY)
        : _elems(nullptr),
          _head(0),
          _tail(0),
          _size(0),
          _capacity(capacity)
    {
        // Capacity must not be zero
        assert(_capacity > 0);
    }

    /**
     * @brief Destructor
     */
    ~queue() {
        if (_elems)
            free(_elems);
    }

    /**
     * @brief Add an element to the end of the queue. The queue will double in capacity of there is
     * not enough room.
     *
     * @param elem Element to enqueue
     *
     * @return False if memory allocation fails, or true otherwise. When enqueue fails, the queue
     * is not modified.
     */
    bool enqueue(T elem) {
        // Resize or initially allocate the queue if needed
        if (_size + 1 > _capacity || !_elems) {
            unsigned int old_capacity = _capacity;

            while (_size + 1 > _capacity)
                _capacity *= 2;

            T *new_elements = (T *)malloc(sizeof(T) * _capacity);

            // If allocation fails, return an error
            if (!new_elements) {
                _capacity = old_capacity;
                return false;
            }

            // TODO: Might want to realloc where possible instead of always copying, in case the
            // allocator can just extend in place.

            // Copy old elements into new element buffer if needed
            if (_elems) {
                // Queue does not wrap around, a single copy is sufficient. Compact to beginning of
                // new queue for simplicity. We will never encounter _head == _tail because otherwise
                // we wouldnt be expanding the queue.
                if (_tail > _head) {
                    memcpy(new_elements, _elems + _head, _size * sizeof(T));
                    _head = 0;
                    _tail = _size;
                }
                // Otherwise, queue wraps around, so we need two copies. Copy to beginning of new
                // queue so that everything is guaranteed to fit without wrapping, since we've
                // doubled the size.
                else {
                    // Size of chunks at the end and beginning of array, due to wrap around
                    unsigned int end_size = old_capacity - _head;
                    unsigned int begin_size = _tail;

                    memcpy(new_elements, _elems + _head, end_size * sizeof(T));
                    memcpy(new_elements + end_size, _elems, begin_size * sizeof(T));

                    _head = 0;
                    _tail = begin_size + end_size;
                }

                // Delete the old queue
                free(_elems);
            }

            _elems = new_elements;
        }

        _elems[_tail] = elem;

        _tail = (_tail + 1) & (_capacity - 1); // Queue capacity is always a power of two
        _size++;

        return true;
    }

    /**
     * @brief Get the first element in the queue. If the queue is empty, behavior is undefined.
     * Therefore, it is the responsibility of the caller to first check if the queue is empty. This
     * allows the implementation to avoid relying on pointers, exceptions, etc. for performance
     * reasons.
     *
     * @return First element in queue, assuming it is not empty
     */
    inline T dequeue() {
        T elem = _elems[_head];

        _head = (_head + 1) & (_capacity - 1); // Queue capacity is always a power of two
        _size--;

        return elem;
    }

    /**
     * @brief Check whether the queue is empty
     */
    inline bool empty() {
        return _size == 0;
    }

    /**
     * @brief Get number of elements in queue
     */
    inline unsigned int size() {
        return _size;
    }

    /**
     * @brief Empty the queue
     */
    inline void clear() {
        _head = 0;
        _tail = 0;
        _size = 0;
    }

};

}

#endif
