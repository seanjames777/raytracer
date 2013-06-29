/*
 * Sean James
 *
 * thread.cpp
 *
 * A thread class that provides object oriented access to posix threads
 *
 */

#include "thread.h"

/*
 * Empty constructor will not create a valid thread, only useful for
 * array initialization.
 */
Thread::Thread()
{
}

/*
 * Standard constructor accepts a function to call as the main
 * method of the new thread and an argument to that function.
 */
Thread::Thread(void *(*Callback)(void *), void *Arg) 
	: callback(Callback), arg(Arg)
{
}

/*
 * Start executing the thread
 */
void Thread::start() {
	if (pthread_create(&thread, NULL, callback, arg) != 0) {
		printf("pthread_create failed\n");
		//exit(0);
	}
}

/*
 * Wait for the thread to terminate and get the value returned
 * by its main method or thread_exit()
 */
void *Thread::join() {
	void *ret = NULL;
	if (pthread_join(thread, &ret) != 0) {
		printf("pthread_join failed\n");
		//exit(0);
	}

	return ret;
}
	
/*
 * Immediately terminate the thread
 */
void Thread::kill() {
	if (pthread_cancel(thread) != 0) {
		printf("pthread_cancel failed\n");
		//exit(0);
	}
}

/*
 * Set the thread to run in detached mode. Will not be joinable
 * but can safely be forgotten about.
 */
void Thread::detach() {
	if (pthread_detach(thread) != 0) {
		printf("pthread_detach failed\n");
		//exit(0);
	}
}
