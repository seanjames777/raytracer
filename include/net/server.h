/**
 * @file server.h
 *
 * @brief Generic platform independent TCP server
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __NET_SERVER_H
#define __NET_SERVER_H

#include <net.h>

// TODO: Windows shutdown()

/**
 * @brief Generic TCP server. Simply binds to a socket and accepts connections.
 * Derived classes are responsible for handling connections.
 */
class Server {
private:

	bool            should_shutdown; //!< Whether the server should shut down
	std::atomic_int num_connections; //!< Number of clients connected

	/**
	 * @brief Start listening for client connections
	 *
	 * @param[in]  port       Port to listen on
	 * @param[out] sockfd_out On success, server socket
	 *
	 * @return True on success, or false on error
	 */
	bool start_server(int port, socket_t *sockfd_out) {
#ifdef _WIN32
		WSADATA wsaData;
		SOCKET sockfd = INVALID_SOCKET;
		struct addrinfo *result = NULL;
		struct addrinfo hints;

		// Initialize WinSock
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			printf("Error initializing WinSock\n");
			return false;
		}

		memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Convert port to string
		char port_str[33];
		memset(port_str, 0, 33);
		itoa(port, port_str, 10);

		// Resolve server address/port
		if ((getaddrinfo(NULL, port_str, &hints, &result)) != 0) {
			printf("Failed to resolve server address/port\n");
			WSACleanup();
			return false;
		}

		// Create a socket to listen on
		sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

		if (sockfd == INVALID_SOCKET) {
			printf("Failed to open socket for listening\n");
			freeaddrinfo(result);
			WSACleanup();
			return false;
		}

		int opt = 1;

		// Allow immediate reuse of address for debugging
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)) == SOCKET_ERROR) {
			printf("Error setting SO_REUSEADDR\n");
			freeaddrinfo(result);
			closesocket(sockfd);
			WSACleanup();
			return false;
		}

		// Bind listening socket
		if (bind(sockfd, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
			printf("Failed to bind socket for listening\n");
			freeaddrinfo(result);
			closesocket(sockfd);
			WSACleanup();
			return false;
		}

		freeaddrinfo(result);

		// Listen for connections
		if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR) {
			printf("Failed to listen on socket\n");
			closesocket(sockfd);
			WSACleanup();
		}

		printf("Listening on port %d\n", port);
#else
		int sockfd;

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("Error creating socket\n");
			return false;
		}

		int opt = 1;

		// Allow immediate reuse of address for debugging
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
			printf("Error setting SO_REUSEADDR\n");
			close(sockfd);
			return false;
		}

		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(port);

		if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			printf("Error binding socket\n");
			close(sockfd);
			return false;
		}

		// TODO: 32
		if (listen(sockfd, 32) < 0) {
			printf("Error listening on socket\n");
			close(sockfd);
			return false;
		}
#endif

		*sockfd_out = sockfd;

		return true;
	}

	/**
	 * @brief Accept a client connection
	 *
	 * @param[in]  sockfd    Server socket
	 * @param[out] clifd_out On success, client socket
	 *
	 * @return True on success, or false on error
	 */
	bool accept_client(socket_t sockfd, socket_t *clifd_out) {
#ifdef _WIN32
		SOCKET clifd = INVALID_SOCKET;

		if ((clifd = accept(sockfd, NULL, NULL)) == INVALID_SOCKET) {
			printf("Error accepting client\n");
			return false;
		}

		printf("Connected to client\n");
#else
		struct sockaddr_in cli_addr;
		socklen_t clilen = sizeof(cli_addr);
		int clifd;

		if ((clifd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0) {
			printf("Error accepting client\n");
			return false;
		}

		printf("Connected to client\n");
#endif

		*clifd_out = clifd;
		return true;
	}

	/**
	 * @brief Disconnect from a client
	 *
	 * @param[in] clifd Client socket
	 */
	void stop_client(socket_t clifd) {
#ifdef _WIN32
		closesocket(clifd);
#else
		close(clifd);
#endif
	}

	/**
	 * @brief Stop listening for client connections
	 *
	 * @param[in] sockfd Server socket
	 */
	void stop_server(socket_t sockfd) {
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
	}

	/**
	 * @brief Worker thread entrypoint to handle a client connection.
	 * The worker indicates that is has finished by decrementing num_connections.
	 */
	void handle_client_worker_thread(socket_t clifd) {
		handle_client(clifd);

		stop_client(clifd);

		num_connections--;
	}

protected:

	/**
	 * @brief Notify the server that it should stop accepting new
	 * connections. When all clients have disconnected, it will
	 * shut down the server.
	 */
	void setShouldShutDown() {
		should_shutdown = true;
	}

	/**
	 * @brief Handle a client connection. Derived classes should override
	 * this function, using write_buff and read_buff to communicate with
	 * the client. This function is run on its own thread. When it returns,
	 * the connection with the client will be closed. To signal that the
	 * server should shut down, call setShouldShutDown(). If an error
	 * occurs, the function should return.
	 *
	 * @param[in] clifd Client socket
	 */
	virtual void handle_client(socket_t clifd) {
	}

public:

	/**
	 * @brief Constructor
	 */
	Server() {
	}

	/**
	 * @brief Destructor
	 */
	~Server() {
	}

	/**
	 * @brief Listen for and handle incoming connections. This function may be
	 * called multiple times, but by only one thread at a time.
	 *
	 * @param port Port to listen on
	 *
	 * @return Returns false on error, blocks on success until shutdown, then
	 * returns true.
	 */
	bool serve(int port) {
		should_shutdown = false;
		num_connections = 0;

		socket_t sockfd;
		
		if (!start_server(port, &sockfd))
			return false;

		while (!should_shutdown) {
			socket_t clifd;

			if (!accept_client(sockfd, &clifd))
				break;

			num_connections++;

			// Create a new thread to handle the connection. The thread will be
			// detached and run on its own.
			std::thread worker(
				std::bind(&Server::handle_client_worker_thread, this, clifd));
			worker.detach();
		}

		while (num_connections > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			// TODO spinning is bad mmk
		}

		stop_server(sockfd);

		return true;
	}
};

#endif
