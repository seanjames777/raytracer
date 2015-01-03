/**
 * @file timer.h
 *
 * @brief Timer utility class
 *
 * @author Sean James
 */

#ifndef _TIMER_H
#define _TIMER_H

#include <sys/time.h>
#include <sys/resource.h>

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

    /**
     * @brief Get the total CPU time used by the process in milliseconds
     */
    double getCPUTime();

};

#endif
