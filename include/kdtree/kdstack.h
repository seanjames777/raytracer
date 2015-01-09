/**
 * @file kdstack.h
 *
 * @brief Reusable KD tree traversal stack structure. KD trees are traversed recursively. For
 * performance, however, a stack data structure can be used instead of actual recursion to avoid
 * function call overhead. The KDStack structure further improves performance by
 * allowing clients to provide a reusable stack structure, instead of allocating memory for
 * a stack every traversal.
 *
 * @author Sean James
 */

#ifndef _KDSTACK_H
#define _KDSTACK_H

#include <kdtree/kdnode.h>

#define KD_INIT_STACK_SIZE 64

// TODO:
//     - Align the stack

/**
 * @brief KD-tree traversal stack item
 */
struct KDStackFrame {
    /** @brief KD-Node to traverse */
    KDNode *node;

    /** @brief Entry distance from ray origin */
    float enter;

    /** @brief Exit distance from ray origin */
    float exit;
};

/**
 * @brief KD-Tree traversal stack
 */
class KDStack {
private:

    KDStackFrame *stack;
    int capacity;
    int size;

public:

    KDStack()
        : capacity(KD_INIT_STACK_SIZE),
          size(0)
    {
        // TODO: handle failure
        // Note: KDStackFrame is POD, so we don't need to worry about constructors
        stack = (KDStackFrame *)malloc(sizeof(KDStackFrame) * KD_INIT_STACK_SIZE);
    }

    ~KDStack() {
        free(stack);
    }

    /**
     * @brief Add a stack frame to the top of the stack, allocating more memory if needed. Returns
     * the newly allocated frame.
     */
    inline KDStackFrame *push() {
        // Note: this rarely happens, so the branch predictor should be happy
        if (size == capacity) {
            // Double for amortized constant cost. In reality, we probably won't even need to
            // double too often. The first few rays will hopefully set the stack size, and the
            // default size may be enough for many scenes. TODO make sure that's true.
            capacity <<= 1;

            // TODO: handle failure
            stack = (KDStackFrame *)realloc(stack, sizeof(KDStackFrame) * capacity);
        }

        return &stack[size++];
    }

    /**
     * @brief Pop the top frame off of the stack. Returns null if there are no more frames.
     */
    inline KDStackFrame *pop() {
        // Note: This rarely happens, so branch predictor should be happy here as well
        if (size == 0)
            return nullptr;

        return &stack[--size];
    }

};

#endif
