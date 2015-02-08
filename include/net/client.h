/**
* @file client.h
*
* @brief Generic platform independent TCP client
*
* @author Sean James <seanjames777@gmail.com>
*/

#ifndef __NET_CLIENT_H
#define __NET_CLIENT_H

#include <net.h>

// TODO: Handle attempts to connect multiple times, etc.
// TODO: Move net.h, namespacing, call these NetTCPClient or something

/**
 * @brief Generic TCP client
 */
class Client {
private:

	socket_t sockfd;

	/**
	 * @brief Connect client to server
	 *
	 * @param[in] host        Host name to connect to
	 * @param[in] port        Host port to connect to
	 * @param[out] sockfd_out On success, connection socket
	 *
	 * @return True on success, or false on error
	 */
	bool connect_client(std::string host, int port, socket_t *sockfd_out) {
#ifdef _WIN32
		WSADATA wsaData;
		SOCKET sockfd;

		struct addrinfo *result = NULL;
		struct addrinfo hints;

		// Start Winsock
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			printf("Failed to start winsock\n");
			return false;
		}

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Convert port to string
		char port_str[33];
		memset(port_str, 0, 33);
		itoa(port, port_str, 10);

		// Resolve host and port
		if (getaddrinfo(host.c_str(), port_str, &hints, &result) != 0) {
			printf("Failed to resolve host/port\n");
			WSACleanup();
			return false;
		}

		// Try to connect to addresses until one succeeds
		for (struct addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			// Create a socket
			sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

			if (sockfd == INVALID_SOCKET) {
				printf("Failed to create socket\n");
				freeaddrinfo(result);
				WSACleanup();
				return false;
			}

			// Try to connect to server
			if (connect(sockfd, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
				closesocket(sockfd);
				sockfd = INVALID_SOCKET;
				continue;
			}

			break;
		}

		freeaddrinfo(result);

		if (sockfd == INVALID_SOCKET) {
			printf("Unable to connect to server\n");
			WSACleanup();
			return false;
		}

		printf("Connected to server\n");
#else
		// TODO: comment

		int sockfd;
		struct hostent *server;
		server = gethostbyname(addr.c_str());

		if (server == NULL) {
			printf("Error parsing address string: %s\n", addr.c_str());
			return false;
		}

		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		serv_addr.sin_port = htons(port);

		bool connected = false;
		int attempts = 0;

		while (!connected && attempts++ < 15) {
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				printf("Error creating socket\n");
				return false;
			}

			if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
				close(sockfd);
				usleep(1000 * 1000);
				continue;
			}

			connected = true;
			break;
		}

		if (!connected) {
			printf("Error connecting to worker: %s:%d\n", addr.c_str(), port);
			close(sockfd);
			return false;
		}

		// TODO: Print stuff on windows maybe
		printf("Connected to worker: %s:%d\n", addr.c_str(), port);
#endif

		*sockfd_out = sockfd;

		return true;
	}

	/**
	 * @brief Disconnect the client from the server
	 *
	 * @param[in] sockfd Connection socket
	 */
	void stop_client(socket_t sockfd) {
#ifdef WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
	}

protected:

	/**
	 * @brief Get server socket
	 */
	socket_t getSocket() {
		return sockfd;
	}

public:

	/**
	 * @brief Constructor
	 */
	Client() {
	}

	/**
	 * @brief Destructor
	 */
	~Client() {
	}

	/**
	 * @brief Connect to a server. This may be called multiple times, but only
	 * from one thread at a time.
	 *
	 * @param[in] host Host address
	 * @param[in] port Host port
	 *
	 * @return True on success, or false on error
	 */
	bool connect_to_server(std::string host, int port) {
		return connect_client(host, port, &sockfd);
	}

	/**
	 * @brief Close the connectio to the server
	 */
	void disconnect_from_server() {
		stop_client(sockfd);
	}

};

#endif
