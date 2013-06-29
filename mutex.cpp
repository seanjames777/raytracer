/*
 * Sean James
 *
 * mutex.cpp
 *
 * A mutex class that provides an object oriented wrapper around
 * posix semaphores. Allows only one thread at a time to access
 * a resource.
 *
 */

#include "mutex.h"

/*
 * Constructor initializes underlying semaphore to 1
 */
Mutex::Mutex() {
	if (sem_init(&sem, false, 1) < 0) {
		printf("ERROR: sem_init failed: %s\n", strerror(errno));
		//exit(0);
	}
}

/*
 * Destroy the underlying semaphore
 */
Mutex::~Mutex() {
	if (sem_destroy(&sem) < 0) {
		printf("ERROR: sem_destroy failed: %s\n", strerror(errno));
		//exit(0);
	}
}

/*
 * Wait for sole access to a shared resource. Conceptually, atomic:
 * [ while(sem == 0) {} wait--; ]
 */
void Mutex::acquire() {
	if (sem_wait(&sem) < 0) {
		printf("ERROR: sem_wait failed: %s\n", strerror(errno));
		//exit(0);
	}
}

/*
 * Release access to a shared resource. Conceptually, atomic [ sem++ ]
 */
void Mutex::release() {
	if (sem_post(&sem) < 0) {
		printf("ERROR: sem_post failed: %s\n", strerror(errno));
		//exit(0);
	}
}
