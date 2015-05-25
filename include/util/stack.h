/**
 * @file kdstack.h
 *
 * @brief Reusable KD tree traversal stack structure. KD trees are traversed recursively. For
 * performance, however, a stack data structure can be used instead of actual recursion to avoid
 * function call overhead. The util::stack<KDStackFrame> structure further improves performance by
 * allowing clients to provide a reusable stack structure, instead of allocating memory for
 * a stack every traversal.
 *
 * @author Sean James
 */

#ifndef _UTIL_STACK_H
#define _UTIL_STACK_H

#include <stdlib.h>
#include <string.h>
#include <cassert>

// Default initial stack capacity
#define DEFAULT_INIT_STACK_CAPACITY 1024

// TODO:
//     - Align the stack
//     - We actually need to call constructors and destructors
//     - Use bigger sizes, i.e. size_t
//     - Inlining decisions

namespace util {

/**
 * @brief Stack
 */
template<typename T>
class stack {
private:

    T   *_stack;
    int  _capacity;
    int  _size;

public:

    stack(unsigned int capacity = DEFAULT_INIT_STACK_CAPACITY)
        : _stack(nullptr),
          _capacity(capacity),
          _size(0)
    {
        // TODO: handle failure

        // Note: util::stack<KDStackFrame>Frame is POD, so we don't need to worry about constructors TODO
        // _stack = (T *)malloc(sizeof(T) * _capacity);

        posix_memalign((void **)&_stack, alignof(T), sizeof(T) * _capacity);
    }

    ~stack() {
        if (_stack)
            free(_stack);
    }

    /**
     * @brief Add a stack frame to the top of the stack, allocating more memory if needed. Returns
     * the newly allocated frame.
     */
    inline void push(const T & elem) {
#if 0
        // TODO: Removing this branch improves performance by about 3%-4%, but we need to make sure
        // to allocate for the worst case.
        // Note: this rarely happens, so the branch predictor should be happy
        if (_size == _capacity) {
            // Double for amortized constant cost. In reality, we probably won't even need to
            // double too often. The first few rays will hopefully set the stack size, and the
            // default size may be enough for many scenes. TODO make sure that's true.
            _capacity <<= 1;

            // TODO: handle failure
            _stack = (T *)realloc(_stack, sizeof(T) * _capacity);
        }
#endif

        _stack[_size++] = elem; // TODO: Copy
    }

    /**
     * @brief Pop the top frame off of the stack. The caller should check empty()
     * first. TODO.
     */
    inline T pop() {
        return _stack[--_size];
    }

    /**
     * @brief Get number of elements in stack
     */
    inline unsigned int size() const {
        return _size;
    }

    /**
     * @brief Check whether the stack is empty
     */
    inline bool empty() const {
        return _size == 0;
    }

    /**
     * @brief Clear the stack
     */
    inline void clear() {
        _size = 0;
    }

};

}

#endif
