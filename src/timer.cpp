/**
 * @file timer.cpp
 *
 * @author Sean James
 */

#include <timer.h>

Timer::Timer() {
	gettimeofday(&startTime, NULL);
}

void Timer::reset() {
	gettimeofday(&startTime, NULL);
}

double Timer::getElapsedMilliseconds() {
	timeval now;
	gettimeofday(&now, NULL);
	double diff = 0.0;

	diff += (now.tv_sec - startTime.tv_sec) * 1000.0;
	diff += (now.tv_usec - startTime.tv_usec) / 1000.0;

	return diff;
}

double Timer::getCPUTime() {
    rusage usage;
    ASSERT(getrusage(RUSAGE_SELF, &usage) == 0);

    double time = 0.0f;

    time += usage.ru_utime.tv_sec * 1000.0;
    time += usage.ru_utime.tv_usec / 1000.0;

    return time;
}