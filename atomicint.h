/*
 * Sean James
 *
 * atomicint.h
 *
 * A wrapper for an integer that supports atomic operations
 *
 */

#ifndef _ATOMICINT_H
#define _ATOMICINT_H

#include "defs.h"
#include "thread.h"
#include "mutex.h"

#include <pthread.h>
#include <semaphore.h>

/*
 * A wrapper for an integer that supports atomic operations
 */
class AtomicInt {
private:

	int value;   // Value of the integer
	Mutex lock;  // Mutex to limit access to one thread

public:

	/*
	 * Default constructor specifies the value of the integer
	 */
	AtomicInt(int Value);

	/*
	 * Get the value of the integer
	 */
	int get();

	/*
	 * Set the value of the integer
	 */
	void set(int Value);

	/*
	 * Atomically postfix increment the integer (x++)
	 */
	int postIncrement();

	/*
	 * Atomically postfix decrement the integer (x--)
	 */
	int postDecrement();
};

#endif
