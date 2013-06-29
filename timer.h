/*
 * Sean James
 *
 * timer.h
 *
 * Timer utility class
 *
 */

#ifndef _TIMER_H
#define _TIMER_H

#include "defs.h"
#include "loggers.h"

#include <ctime>

// Windows does not support clock_gettime
#ifdef WINDOWS

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED

struct timespec {
	long long tv_sec;
	long long tv_nsec;
};

#endif

struct timespec clock_gettime(int type);

#define CLOCK_REALTIME 0

#endif

/*
 * Timer utility class
 */
class Timer {
private:

	struct timespec startTime; // Time the timer was started
	struct timespec stopTime;  // Time the timer was stopped

public:

	/*
	 * Constructor
	 */
	Timer();

	/*
	 * Get the current time in seconds. Useful for initializing random
	 * and generating filenames.
	 */
	static long long getCurrentTimeSeconds();

	/*
	 * Start the timer
	 */
	void start();

	/*
	 * Stop the timer
	 */
	void stop();

	/*
	 * Get the difference between the stop and start times in seconds
	 */
	long long getElapsedSeconds();

	/*
	 * Get the difference between the stop and start times in nanoseconds
	 */
	long long getElapsedNanoseconds();

	/*
	 * Use the main logger to log the elapsed time
	 */
	void print();

	/*
	 * Use the main logger to log the elapsed time with a specific label
	 * e.g. 'render'.
	 */
	void print(string label);

	/*
	 * Stop the timer and print the elapsed time
	 */
	void stopAndPrint();

	/*
	 * Stop the timer and print the elapsed time with a specific label,
	 * e.g. 'render'.
	 */
	void stopAndPrint(string label);
};

#endif