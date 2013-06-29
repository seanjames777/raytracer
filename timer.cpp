/*
 * Sean James
 *
 * timer.cpp
 *
 * Timer utility class
 *
 */

#include "timer.h"

#ifdef WINDOWS

#include <windows.h>

struct timespec clock_gettime(int type) {
	LARGE_INTEGER time;
	LARGE_INTEGER freq;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);

	long long seconds = (long long)((double)time.QuadPart / ((double)freq.QuadPart));
	long long nanoseconds = (long long)((double)time.QuadPart / ((double)freq.QuadPart / 1000000000.0));

	struct timespec t;
	t.tv_sec = seconds;
	t.tv_nsec = nanoseconds;

	return t;
}

#endif

/*
 * Constructor
 */
Timer::Timer() {
}

/*
 * Get the current time in seconds. Useful for initializing random
 * and generating filenames.
 */
long long Timer::getCurrentTimeSeconds() {
	struct timespec now = clock_gettime(CLOCK_REALTIME);
	return now.tv_sec;
}

/*
 * Start the timer
 */
void Timer::start() {
	startTime = clock_gettime(CLOCK_REALTIME);
}

/*
 * Stop the timer
 */
void Timer::stop() {
	stopTime = clock_gettime(CLOCK_REALTIME);
}

/*
 * Get the difference between the stop and start times in seconds
 */
long long Timer::getElapsedSeconds() {
	long long diff = stopTime.tv_sec - startTime.tv_sec;
	return diff;
}

/*
 * Get the difference between the stop and start times in nanoseconds
 */
long long Timer::getElapsedNanoseconds() {
	long long diff = stopTime.tv_nsec - startTime.tv_nsec;
	return diff;
}

/*
 * Use the main logger to log the elapsed time
 */
void Timer::print() {
	Loggers::Main.Log(LOGGER_TIMERS, "Elapsed: %lld seconds, %lld milliseconds\n", getElapsedSeconds(), getElapsedNanoseconds() / 1000000);
}

/*
 * Use the main logger to log the elapsed time with a specific label
 * e.g. 'render'.
 */
void Timer::print(string label) {
	Loggers::Main.Log(LOGGER_TIMERS, "Elapsed (%s): %lld seconds, %lld milliseconds\n", label.c_str(), getElapsedSeconds(), getElapsedNanoseconds() / 1000000);
}

/*
 * Stop the timer and print the elapsed time
 */
void Timer::stopAndPrint() {
	stop();
	print();
}

/*
 * Stop the timer and print the elapsed time with a specific label,
 * e.g. 'render'.
 */
void Timer::stopAndPrint(string label) {
	stop();
	print(label);
}
