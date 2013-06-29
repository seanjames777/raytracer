/*
 * Sean James
 *
 * threadpool.h
 *
 * A thread pool class that spawns and manages the allocation of tasks
 * to a specific pool of threads.
 *
 */

#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "defs.h"
#include "thread.h"
#include "mutex.h"
#include "semaphore.h"

#include <pthread.h>
#include <semaphore.h>

/*
 * A task that will be queued and allocated to a single thread
 */
struct Task {
	void *(*callback)(void *); // function to call
	void *arg;                 // argument to the function
};

/*
 * A thread pool class that spawns and manages the allocation of tasks
 * to a specific pool of threads.
 */
class ThreadPool {
private:

	Task *tasks;           // Circular task buffer
	Thread *threads;       // Pool of threads

	Semaphore fillCount;   // Semaphore indicating tasks are available
	Semaphore emptyCount;  // Semaphore indicating empty task slots are available
	Mutex bufferLock;      // Mutex used to lock the task buffer

	int head;              // The index of the next work item to be started
	int tail;              // The index of the next free task slot
	int taskCapacity;      // The size of the task buffer
	int numThreads;        // The number of threads in the pool

	/*
	 * Function executed by worker threads. Must be terminated by
	 * pthread_cancel (TODO make that better?)
	 */
	static void *worker(void *arg);

public:

	/*
	 * Default constructor accepts number of threads, size of task buffer, and
	 * whether threads should be started immediately
	 */
	ThreadPool(int NumThreads, int TaskCapacity, bool startImmediately);

	/*
	 * Destructor kills worker threads and cleans up memory
	 */
	~ThreadPool();

	/*
	 * Start executing the tasks in the buffer
	 */
	void start();

	/*
	 * Immediately kill all worker threads
	 */
	void kill();

	/*
	 * Submit a task to the pool. Will block if the buffer
	 * is full
	 */
	void submit(void *(*Callback)(void *), void *Arg);

	/*
	 * Get the number of threads in the thread pool
	 */
	int getNumThreads();

	/*
	 * Get the maximum capacity of the task buffer
	 */
	int getTaskCapacity();
};

#endif
