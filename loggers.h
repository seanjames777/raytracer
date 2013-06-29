/*
 * Sean James
 *
 * loggers.h
 *
 * Container class for default loggers
 *
 */

#ifndef _LOGGERS_H
#define _LOGGERS_H

#include "logger.h"

/*
 * Container class for default loggers
 */
class Loggers {
public:

	/*
	 * Main default loggers. Prints to the console only by default. Default
	 * print and file verbosity are 5 and 4 respectively.
	 */
	static Logger Main;
};

#endif
