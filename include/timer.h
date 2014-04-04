/**
 * @file timer.h
 *
 * @brief Timer utility class
 *
 * @author Sean James
 */

#ifndef _TIMER_H
#define _TIMER_H

#include <defs.h>
#include <sys/time.h>

/**
 * @brief Timer utility class
 */
class Timer {
private:

	/** @brief Time the timer was started */
	timeval startTime;

public:

	/**
	 * @brief Constructor
	 */
	Timer();

	/**
	 * @brief Reset the timer
	 */
	void reset();

	/**
	 * @brief Get the difference between the stop and start times in milliseconds
	 */
	double getElapsedMilliseconds();

};

#endif