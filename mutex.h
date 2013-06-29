/*
 * Sean James
 *
 * mutex.h
 *
 * A mutex class that provides an object oriented wrapper around
 * posix semaphores. Allows only one thread at a time to access
 * a resource.
 *
 */

#ifndef _MUTEX_H
#define _MUTEX_H

#include "defs.h"

#include <pthread.h>
#include <semaphore.h>

/*
 * A mutex class that provides an object oriented wrapper around
 * posix semaphores. Allows only one thread at a time to access
 * a resource.
 */
class Mutex {
	sem_t sem; // Posix semaphore

public:

	/*
	 * Constructor initializes underlying semaphore to 1
	 */
	Mutex();

	/*
	 * Destroy the underlying semaphore
	 */
	~Mutex();

	/*
	 * Wait for sole access to a shared resource. Conceptually, atomic:
	 * [ while(sem == 0) {} wait--; ]
	 */
	void acquire();

	/*
	 * Release access to a shared resource. Conceptually, atomic [ sem++ ]
	 */
	void release();
};

#endif