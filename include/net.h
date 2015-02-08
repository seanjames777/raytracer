/**
 * @file net.h
 *
 * @brief Network library definitions and common functions
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __NET_H
#define __NET_H

// TODO: distrubute to .cpp files
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <stdlib.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <string>

#define BUFF_SIZE 4096 // Read/write buffer size

/*
* @brief Generic socket type
*/
#ifdef _WIN32
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif

/**
* @brief Write data to a socket. The write will be buffered such that
* the entire message will be sent, regardless of length, unless an
* error occurs.
*
* @param[in] fd   Socket to write to
* @param[in] buff Buffer to write
* @param[in] len  Number of bytes to write
*
* @return True on success, or false on error
*/
bool write_buff(socket_t fd, char *buff, int len) {
	while (len > 0) {
		int write_count = send(fd, buff, len, 0);

		if (write_count < 0)
			return false;

		buff += write_count;
		len -= write_count;
	}

	return true;
}

/**
* @brief Read data from a socket. The read will be buffered such that
* the entire message will be read, regardless of length, unless an
* error occurs.
*
* @param[in] fd   Socket to read from
* @param[in] buff Buffer to read into
* @param[in] len  Number of bytes to read
*
* @return True on success, or false on error
*/
bool read_buff(socket_t fd, char *buff, int len) {
	while (len > 0) {
		int read_count = send(fd, buff, len, 0);

		if (read_count < 0)
			return false;

		buff += read_count;
		len -= read_count;
	}

	return true;
}

#endif
