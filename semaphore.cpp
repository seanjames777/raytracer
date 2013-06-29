/*
 * Sean James
 *
 * semaphore.cpp
 *
 * A semaphore class that provides an object oriented wrapper around
 * posix semaphores
 *
 */

#include "semaphore.h"

/*
 * Default constructor does not create a valid semaphore, only useful
 * for array initialization
 */
Semaphore::Semaphore() 
{
}

/*
 * Create a semaphore with the given initial value
 */
Semaphore::Semaphore(int init) 
{
	if (sem_init(&sem, false, init)) {
		printf("ERROR: sem_init failed: %s\n", strerror(errno));
		//exit(0);
	}
}

/*
 * Destroy the semaphore
 */
Semaphore::~Semaphore() {
	if (sem_destroy(&sem) < 0) {
		printf("ERROR: sem_destroy failed: %s\n", strerror(errno));
		//exit(0);
	}
}

/*
 * Wait until a resource is available. Conceptually: atomic
 * [ while (sem == 0) {} s--; ]
 */
void Semaphore::wait() {
	if (sem_wait(&sem) < 0) {
		printf("ERROR: sem_wait failed: %s\n", strerror(errno));
		//exit(0);
	}
}

/*
 * Release a resource. Conceptually: atomic [ sem++ ]
 */
void Semaphore::post() {
	if (sem_post(&sem)) {
		printf("ERROR: sem_post failed: %s\n", strerror(errno));
		//exit(0);
	}
}
