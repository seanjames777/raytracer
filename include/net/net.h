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
#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
	#include <unistd.h>
#endif

#include <stdio.h>
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
bool write_buff(socket_t fd, char *buff, int len);

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
bool read_buff(socket_t fd, char *buff, int len);

#endif
