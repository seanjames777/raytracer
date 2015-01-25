/**
 * @file timer.cpp
 *
 * @author Sean James
 */

#include <timer.h>
#include <cassert>

Timer::Timer() {
	reset();
}

void Timer::reset() {
#ifndef _WINDOWS
	gettimeofday(&startTime, nullptr);
#else
	LARGE_INTEGER time, frequency;
	QueryPerformanceCounter(&time);
	QueryPerformanceFrequency(&frequency);

	this->startTime = (double)time.QuadPart;
	this->frequency = (double)frequency.QuadPart;
#endif
}

double Timer::getElapsedMilliseconds() {
#ifndef _WINDOWS
    timeval now;
    gettimeofday(&now, nullptr);
    double diff = 0.0;

    diff += (now.tv_sec - startTime.tv_sec) * 1000.0;
    diff += (now.tv_usec - startTime.tv_usec) / 1000.0;

    return diff;
#else
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

	double ticks = (double)time.QuadPart - startTime;
	return 1000.0 * ticks / frequency;
#endif
}

double Timer::getCPUTime() {
#ifndef _WINDOWS
    rusage usage;
	// TODO: assert may not be included in release
    assert(getrusage(RUSAGE_SELF, &usage) == 0);

    double time = 0.0f;

    time += usage.ru_utime.tv_sec * 1000.0;
    time += usage.ru_utime.tv_usec / 1000.0;

    return time;
#else
	// TODO
	return 0.0;
#endif
}
