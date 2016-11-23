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

    THREAD T *_stack;
	THREAD T *_curr;

public:

    stack(THREAD T *stack)
        : _stack(stack),
		  _curr(stack)
    {
    }

    /**
     * @brief Add a stack frame to the top of the stack, allocating more memory if needed. Returns
     * the newly allocated frame.
     */
    inline void push(T elem) {
        *(_curr++) = elem;
    }

    /**
     * @brief Pop the top frame off of the stack. The caller should check empty()
     * first. TODO.
     */
    inline T pop() {
		return *(--_curr);
    }

    /**
     * @brief Get number of elements in stack
     */
    inline unsigned int size() const {
		return _curr - _stack;
    }

    /**
     * @brief Check whether the stack is empty
     */
    inline bool empty() const {
        return _curr == _stack;
    }

    /**
     * @brief Clear the stack
     */
    inline void clear() {
		_curr = _stack;
    }

};

}

#endif
