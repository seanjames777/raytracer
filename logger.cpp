/*
 * Sean James
 *
 * logger.cpp
 *
 * Utility class for logging messages to the console or to a file
 *
 */

#include "logger.h"

/*
 * Empty constructor for a default, unnamed logger
 */
Logger::Logger()
	: name("unnamed"), printVerbosity(5), fileVerbosity(5), output(NULL)
{
}

/*
 * Constructor specifying logger name and verbosity levels
 */
Logger::Logger(string Name, int PrintVerbosity, int FileVerbosity) 
	: name(Name), printVerbosity(PrintVerbosity), fileVerbosity(FileVerbosity), output(NULL)
{
}

/*
 * Destructor
 */
Logger::~Logger() {
	if (output != NULL) {
		fflush(output);
		fclose(output);
	}
}

/*
 * Log a message with verbosity level 0 by default, according to the format
 * string in 'format'.
 */
void Logger::Log(char *format, ...) {
#ifdef LOGGING
	va_list args1, args2;
	va_start(args1, format);
	va_copy(args2, args1);
		
	printf("%s:%d> ", name.c_str(), 0);
	vprintf(format, args1);

	if (output != NULL) {
		fprintf(output, "%s:%d> ", name.c_str(), 0);
		vfprintf(output, format, args2);
		fflush(output);
	}

	va_end(args1);
	va_end(args2);
#endif
}

/*
 * Log a message with a specific verbosity level, according to the format
 * string in 'format'.
 */
void Logger::Log(int msgVerbosity, char *format, ...) {
#ifdef LOGGING
	va_list args1, args2;
	va_start(args1, format);
	va_copy(args2, args1);

	if (msgVerbosity <= printVerbosity) {
		printf("%s:%d> ", name.c_str(), msgVerbosity);
		vprintf(format, args1);
	}

	if (output != NULL && msgVerbosity <= fileVerbosity) {
		fprintf(output, "%s:%d> ", name.c_str(), msgVerbosity);
		vfprintf(output, format, args2);
		fflush(output);
	}

	va_end(args1);
	va_end(args2);
#endif
}

/*
 * Get the maximum verbosity for messages printed to the console
 */
int Logger::getPrintVerbosity() {
	return printVerbosity;
}

/*
 * Set the maximum verbosity for messages printed to the console
 */
void Logger::setPrintVerbosity(int Verbosity) {
	printVerbosity = Verbosity;
}

/*
 * Get the maximum verbosity for messages printed to a file
 */
int Logger::getFileVerbosity() {
	return fileVerbosity;
}

/*
 * Set the maximum verbosity for messages printed to a file
 */
void Logger::setFileVerbosity(int Verbosity) {
	fileVerbosity = Verbosity;
}

/*
 * Set the output file, closing the current output if necessary
 */
void Logger::setOutput(string filename) {
	if (output != NULL) {
		fclose(output);
		output = NULL;
	}

	output = fopen(filename.c_str(), "w+");
}
