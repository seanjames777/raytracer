/*
 * Sean James
 *
 * semaphore.h
 *
 * A semaphore class that provides an object oriented wrapper around
 * posix semaphores
 *
 */

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#include "defs.h"

#include <pthread.h>
#include <semaphore.h>

/*
 * A semaphore class that provides an object oriented wrapper around
 * posix semaphores
 */
class Semaphore {
private:

	sem_t sem;        // Posix semaphore
	
public:

	/*
	 * Default constructor does not create a valid semaphore, only useful
	 * for array initialization
	 */
	Semaphore();

	/*
	 * Create a semaphore with the given initial value
	 */
	Semaphore(int init);

	/*
	 * Destroy the semaphore
	 */
	~Semaphore();

	/*
	 * Wait until a resource is available. Conceptually: atomic
	 * [ while (sem == 0) {} s--; ]
	 */
	void wait();

	/*
	 * Release a resource. Conceptually: atomic [ sem++ ]
	 */
	void post();
};

#endif