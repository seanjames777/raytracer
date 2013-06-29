/*
 * Sean James
 *
 * loggers.h
 *
 * Container class for default loggers
 *
 */

#include "loggers.h"

/*
 * Main default loggers. Prints to the console only by default. Default
 * print and file verbosity are 5 and 4 respectively.
 */
Logger Loggers::Main("main", 5, 4);
