/**
 * @file util/kdstack.h
 *
 * @brief Reusable KD tree traversal stack structure. KD trees are traversed recursively. For
 * performance, however, a stack data structure can be used instead of actual recursion to avoid
 * function call overhead. The util::stack<KDStackFrame> structure further improves performance by
 * allowing clients to provide a reusable stack structure, instead of allocating memory for
 * a stack every traversal.
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __STACK_H
#define __STACK_H

namespace util {

/**
 * @brief Stack
 */
template<typename T>
class stack {
private:

#if GPU
    thread T *_stack;
#else
    T *_stack;
#endif
    int  _size;

public:

#if GPU
    stack(thread T *stack)
#else
    stack(T *stack)
#endif
        : _stack(stack),
          _size(0)
    {
    }

    /**
     * @brief Add a stack frame to the top of the stack, allocating more memory if needed. Returns
     * the newly allocated frame.
     */
    inline void push(T elem) {
        _stack[_size++] = elem;
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
