/**
 * @file util/timer.h
 *
 * @brief Timer utility class
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef _TIMER_H
#define _TIMER_H

#include <rt_defs.h>

#ifndef _WIN32
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/time.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

/**
 * @brief Timer utility class
 */
class RT_EXPORT Timer {
private:

    /** @brief Time the timer was started */
#ifndef _WINDOWS
    timeval startTime;
#else
    double startTime;
    double frequency;
#endif

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
     * @brief Get the current time in milliseconds
     */
    double getTime();

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
