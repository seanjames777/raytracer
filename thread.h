/*
 * Sean James
 *
 * thread.h
 *
 * A thread class that provides object oriented access to posix threads
 *
 */

#ifndef _THREAD_H
#define _THREAD_H

#include "defs.h"

#include <pthread.h>
#include <semaphore.h>

// Allow threads to think they're not using pthreads
#define thread_exit(arg) pthread_exit(arg)
#define thread_self()    pthread_self()

// Make windows conform to linux sleep()
#ifdef WINDOWS
#include <windows.h>
#define sleep(secs) Sleep((secs) * 1000)
#endif

/*
 * Thread class that provides an object-oriented wrapper around pthreads
 */
class Thread {
private:

	pthread_t thread;           // Posix threads
	void *(*callback)(void *);  // Thread main function
	void *arg;                  // Thread argument

public: 

	/*
	 * Empty constructor will not create a valid thread, only useful for
	 * array initialization.
	 */
	Thread();

	/*
	 * Standard constructor accepts a function to call as the main
	 * method of the new thread and an argument to that function.
	 */
	Thread(void *(*Callback)(void *), void *Arg);

	/*
	 * Start executing the thread
	 */
	void start();

	/*
	 * Wait for the thread to terminate and get the value returned
	 * by its main method or thread_exit()
	 */
	void *join();
	
	/*
	 * Immediately terminate the thread
	 */
	void kill();

	/*
	 * Set the thread to run in detached mode. Will not be joinable
	 * but can safely be forgotten about.
	 */
	void detach();
};

#endif
