/**
 * @file queue.h
 *
 * @brief Queue/FIFO
 *
 * @author Sean James <seanjames777@mgmail.com>
 */

// TODO: Namespacing for various directories

#ifndef __QUEUE_H
#define __QUEUE_H

#include <stdlib.h>
#include <string.h>
#include <cassert>

// Default initial queue capacity
#define DEFAULT_INIT_QUEUE_CAPACITY 16

/**
 * @brief Generic circular queue/FIFO. Dynamically resizes as needed.
 *
 * @tparam T Element type
 */
template<typename T>
class Queue {
private:

    T            *elems;    // Element buffer
    unsigned int  head;     // First element
    unsigned int  tail;     // One past end of queue, unless it is empty, wrapped around capacity
    unsigned int  size;     // Current number of elements. TODO: can be computed from head/tail
    unsigned int  capacity; // Maximum number of elements

    // Notes:
    //   - We use malloc/free for the elements here, because we overwrite the allocated
    //     space with actual values.
    //   - Initially, capacity is used to indicate the desired capacity, but elems is null.
    //   - Insertion is allowed to fail, signalling out-of-memory.

public:

    /**
     * @brief Constructor
     *
     * @param capacity Desired initial capacity of the queue. Must be greater than 0.
     */
    Queue(unsigned int capacity = DEFAULT_INIT_QUEUE_CAPACITY)
        : elems(nullptr),
          head(0),
          tail(0),
          size(0),
          capacity(capacity)
    {
        // Capacity must not be zero
        assert(capacity > 0);
    }

    /**
     * @brief Destructor
     */
    ~Queue() {
        if (elems)
            free(elems);
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
        if (size + 1 > capacity || !elems) {
            unsigned int old_capacity = capacity;

            while (size + 1 > capacity)
                capacity *= 2;

            T *new_elements = (T *)malloc(sizeof(T) * capacity);

            // If allocation fails, return an error
            if (!new_elements) {
                capacity = old_capacity;
                return false;
            }

            // TODO: Might want to realloc where possible instead of always copying, in case the
            // allocator can just extend in place.

            // Copy old elements into new element buffer if needed
            if (elems) {
                // Queue does not wrap around, a single copy is sufficient. Compact to beginning of
                // new queue for simplicity. We will never encounter head == tail because otherwise
                // we wouldnt be expanding the queue.
                if (tail > head) {
                    memcpy(new_elements, elems + head, size * sizeof(T));
                    head = 0;
                    tail = size;
                }
                // Otherwise, queue wraps around, so we need two copies. Copy to beginning of new
                // queue so that everything is guaranteed to fit without wrapping, since we've
                // doubled the size.
                else {
                    // Size of chunks at the end and beginning of array, due to wrap around
                    unsigned int end_size = old_capacity - head;
                    unsigned int begin_size = tail;

                    memcpy(new_elements, elems + head, end_size * sizeof(T));
                    memcpy(new_elements + end_size, elems, begin_size * sizeof(T));

                    head = 0;
                    tail = begin_size + end_size;
                }

                // Delete the old queue
                free(elems);
            }

            elems = new_elements;
        }

        elems[tail] = elem;

        tail = (tail + 1) & (capacity - 1); // Queue capacity is always a power of two
        size++;

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
    T dequeue() {
        T elem = elems[head];

        head = (head + 1) & (capacity - 1); // Queue capacity is always a power of two
        size--;

        return elem;
    }

    /**
     * @brief Check whether the queue is empty
     */
    bool isEmpty() {
        return size == 0;
    }

    /**
     * @brief Get number of elements in queue
     */
    unsigned int getSize() {
        return size;
    }

};

#endif
