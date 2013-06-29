/*
 * Sean James
 *
 * logger.h
 *
 * Utility class for logging messages to the console or to a file
 *
 */

#ifndef _LOGGER_H
#define _LOGGER_H

#include "defs.h"
#include <stdarg.h>

#ifdef WINDOWS
#define va_copy(a, b) ((a) = (b))
#endif

// Logger levels
#define LOGGER_ERROR       0
#define LOGGER_CRITICAL    1
#define LOGGER_TIMERS      2
#define LOGGER_DEBUG       3
#define LOGGER_NONCRITICAL 4
#define LOGGER_PERCENTAGE  5

// Whether logging is enabled (disabling may improve performance)
#define LOGGING

/*
 * Utility class for logging messages to the console or to a file
 *
 * Conventions:
 *
 * Level 0) Errors
 * Level 1) Important status messages
 * Level 2) Timers
 * Level 3) Debug info
 * Level 4) Non-critical status messages
 * Level 5) Percent complete
 */
class Logger {
private:

	int printVerbosity;  // Max level printed to the console
	int fileVerbosity;   // Max level printed to a file
	FILE *output;        // Output file
	string name;         // Name of this Logger

public:

	/*
	 * Empty constructor for a default, unnamed logger
	 */
	Logger();

	/*
	 * Constructor specifying logger name and verbosity levels
	 */
	Logger(string Name, int PrintVerbosity, int FileVerbosity);

	/*
	 * Destructor
	 */
	~Logger();

	/*
	 * Log a message with verbosity level 0 by default, according to the format
	 * string in 'format'.
	 */
	void Log(char *format, ...);

	/*
	 * Log a message with a specific verbosity level, according to the format
	 * string in 'format'.
	 */
	void Log(int msgVerbosity, char *format, ...);

	/*
	 * Get the maximum verbosity for messages printed to the console
	 */
	int getPrintVerbosity();

	/*
	 * Set the maximum verbosity for messages printed to the console
	 */
	void setPrintVerbosity(int Verbosity);

	/*
	 * Get the maximum verbosity for messages printed to a file
	 */
	int getFileVerbosity();

	/*
	 * Set the maximum verbosity for messages printed to a file
	 */
	void setFileVerbosity(int Verbosity);

	/*
	 * Set the output file, closing the current output if necessary
	 */
	void setOutput(string filename);
};

#endif