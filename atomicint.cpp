/*
 * Sean James
 *
 * atomicint.cpp
 *
 * A wrapper for an integer that supports atomic operations
 *
 */

#include "atomicint.h"

/*
 * Default constructor specifies the value of the integer
 */
AtomicInt::AtomicInt(int Value)
	: value(Value)
{
}

/*
 * Get the value of the integer
 */
int AtomicInt::get() {
	return value;
}

/*
 * Set the value of the integer
 */
void AtomicInt::set(int Value) {
	value = Value;
}

/*
 * Atomically postfix increment the integer (x++)
 */
int AtomicInt::postIncrement() {
	lock.acquire();
	int ret = value;
	value++;
	lock.release();

	return ret;
}

/*
 * Atomically postfix decrement the integer (x--)
 */
int AtomicInt::postDecrement() {
	lock.acquire();
	int ret = value;
	value--;
	lock.release();

	return ret;
}
