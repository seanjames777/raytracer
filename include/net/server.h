/**
 * @file server.h
 *
 * @brief Generic platform independent TCP server
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __NET_SERVER_H
#define __NET_SERVER_H

#include <net/net.h>

// TODO: Windows shutdown()

/**
 * @brief Generic TCP server. Simply binds to a socket and accepts connections.
 * Derived classes are responsible for handling connections.
 */
class Server {
private:

	bool            should_shutdown; //!< Whether the server should shut down
	std::atomic_int num_connections; //!< Number of clients connected
    socket_t        sockfd;          //!< Server socket

	/**
	 * @brief Start listening for client connections
	 *
	 * @param[in]  port       Port to listen on
	 * @param[out] sockfd_out On success, server socket
	 *
	 * @return True on success, or false on error
	 */
	bool start_server(int port, socket_t *sockfd_out);

	/**
	 * @brief Accept a client connection
	 *
	 * @param[in]  sockfd    Server socket
	 * @param[out] clifd_out On success, client socket
	 *
	 * @return True on success, or false on error
	 */
	bool accept_client(socket_t sockfd, socket_t *clifd_out);

	/**
	 * @brief Disconnect from a client
	 *
	 * @param[in] clifd Client socket
	 */
	void stop_client(socket_t clifd);

	/**
	 * @brief Stop listening for client connections
	 *
	 * @param[in] sockfd Server socket
	 */
	void stop_server(socket_t sockfd);

	/**
	 * @brief Worker thread entrypoint to handle a client connection.
	 * The worker indicates that is has finished by decrementing num_connections.
	 */
	void handle_client_worker_thread(socket_t clifd);

protected:

	/**
	 * @brief Handle a client connection. Derived classes should override
	 * this function, using write_buff and read_buff to communicate with
	 * the client. This function is run on its own thread. When it returns,
	 * the connection with the client will be closed. To signal that the
	 * server should shut down, call setShouldShutDown(). If an error
	 * occurs, the function should return. The client should return as soon as
     * possible if shouldShutDown() becomes true.
	 *
	 * @param[in] clifd Client socket
	 */
	virtual void handle_client(socket_t clifd);

public:

	/**
	 * @brief Constructor
	 */
	Server();

	/**
	 * @brief Destructor
	 */
	~Server();

    /**
     * @brief Get whether the server should shut down
     */
    bool shouldShutDown();

    /**
     * @brief Notify the server that it should stop accepting new
     * connections. When all clients have disconnected, it will
     * shut down the server.
     */
    void setShouldShutDown();

	/**
	 * @brief Listen for and handle incoming connections. This function may be
	 * called multiple times, but by only one thread at a time.
	 *
	 * @param port Port to listen on
	 *
	 * @return Returns false on error, blocks on success until shutdown, then
	 * returns true.
	 */
	bool serve(int port);
};

#endif
