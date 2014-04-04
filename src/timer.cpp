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
