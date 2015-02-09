/**
* @file client.h
*
* @brief Generic platform independent TCP client
*
* @author Sean James <seanjames777@gmail.com>
*/

#ifndef __NET_CLIENT_H
#define __NET_CLIENT_H

#include <net/net.h>

// TODO: Handle attempts to connect multiple times, etc.
// TODO: Move net.h, namespacing, call these NetTCPClient or something

/**
 * @brief Generic TCP client
 */
class Client {
private:

	socket_t sockfd; //!< Client connection socket

	/**
	 * @brief Connect client to server
	 *
	 * @param[in] host        Host name to connect to
	 * @param[in] port        Host port to connect to
	 * @param[out] sockfd_out On success, connection socket
	 *
	 * @return True on success, or false on error
	 */
	bool connect_client(std::string host, int port, socket_t *sockfd_out);

	/**
	 * @brief Disconnect the client from the server
	 *
	 * @param[in] sockfd Connection socket
	 */
	void stop_client(socket_t sockfd);

protected:

	/**
	 * @brief Get server socket
	 */
	socket_t getSocket();

public:

	/**
	 * @brief Constructor
	 */
	Client();

	/**
	 * @brief Destructor
	 */
	~Client();

	/**
	 * @brief Connect to a server. This may be called multiple times, but only
	 * from one thread at a time.
	 *
	 * @param[in] host Host address
	 * @param[in] port Host port
	 *
	 * @return True on success, or false on error
	 */
	bool connect_to_server(std::string host, int port);

	/**
	 * @brief Close the connectio to the server
	 */
	void disconnect_from_server();

};

#endif
