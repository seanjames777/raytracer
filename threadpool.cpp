/*
 * Sean James
 *
 * threadpool.cpp
 *
 * A thread pool class that spawns and manages the allocation of tasks
 * to a specific pool of threads.
 *
 */

#include "threadpool.h"

/*
 * Function executed by worker threads. Must be terminated by
 * pthread_cancel (TODO make that better?)
 */
void *ThreadPool::worker(void *arg) {
	ThreadPool *pool = (ThreadPool *)arg;

	while (true) {
		// Wait for a work item
		pool->fillCount.wait();

		// Lock the buffer
		pool->bufferLock.acquire();

		// Get the next task and move the head index
		Task task = pool->tasks[pool->head];
		pool->head = (pool->head + 1) % pool->taskCapacity;

		// Unlock the buffer
		pool->bufferLock.release();

		// Indicate a free slot available
		pool->emptyCount.post();

		// Execute the task
		task.callback(task.arg);
	}

	return 0;
}

/*
 * Default constructor accepts number of threads, size of task buffer, and
 * whether threads should be started immediately
 */
ThreadPool::ThreadPool(int NumThreads, int TaskCapacity, bool startImmediately) 
	: fillCount(0), 
		emptyCount(TaskCapacity), 
		head(0), 
		tail(0), 
		taskCapacity(TaskCapacity), 
		numThreads(NumThreads)
{
	tasks   = (Task *)  malloc(sizeof(Task)   * taskCapacity);
	threads = (Thread *)malloc(sizeof(Thread) * numThreads);

	for (int i = 0; i < numThreads; i++) {
		threads[i] = Thread(&worker, this);
	}

	if (startImmediately)
		start();
}

/*
 * Destructor kills worker threads and cleans up memory
 */
ThreadPool::~ThreadPool() {
	kill();

	free(tasks);
	free(threads);
}

/*
 * Start executing the tasks in the buffer
 */
void ThreadPool::start() {
	for (int i = 0; i < numThreads; i++) {
		threads[i].start();
		threads[i].detach();
	}
}

/*
 * Immediately kill all worker threads
 */
void ThreadPool::kill() {
	for (int i = 0; i < numThreads; i++)
		threads[i].kill();
}

/*
 * Submit a task to the pool. Will block if the buffer
 * is full
 */
void ThreadPool::submit(void *(*Callback)(void *), void *Arg) {
	Task task;
	task.callback = Callback;
	task.arg = Arg;

	emptyCount.wait();

	bufferLock.acquire();
	tasks[tail] = task;
	tail = (tail + 1) % taskCapacity;
	bufferLock.release();

	fillCount.post();
}

/*
 * Get the number of threads in the thread pool
 */
int ThreadPool::getNumThreads() {
	return numThreads;
}

/*
 * Get the maximum capacity of the task buffer
 */
int ThreadPool::getTaskCapacity() {
	return taskCapacity;
}
