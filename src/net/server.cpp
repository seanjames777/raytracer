/**
* @file server.cpp
*
* @author Sean James <seanjames777@gmail.com>
*/

#include <net/server.h>

Server::Server() {
}

Server::~Server() {
}

bool Server::start_server(int port, socket_t *sockfd_out) {
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

bool Server::accept_client(socket_t sockfd, socket_t *clifd_out) {
#ifdef _WIN32
    SOCKET clifd = INVALID_SOCKET;

    if ((clifd = accept(sockfd, NULL, NULL)) == INVALID_SOCKET) {
        printf("Error accepting client\n");
        return false;
    }
#else
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int clifd;

    if ((clifd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0) {
        printf("Error accepting client\n");
        return false;
    }
#endif

    printf("Connected to client\n");

    *clifd_out = clifd;
    return true;
}

void Server::stop_client(socket_t clifd) {
#ifdef _WIN32
    closesocket(clifd);
#else
    close(clifd);
#endif

    printf("Disconnected from client\n");
}

void Server::stop_server(socket_t sockfd) {
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif

    printf("Server shut down\n");
}

void Server::handle_client_worker_thread(socket_t clifd) {
    handle_client(clifd);

    stop_client(clifd);

    num_connections--;
}

bool Server::shouldShutDown() {
    return should_shutdown;
}

void Server::setShouldShutDown() {
    if (should_shutdown)
        return;

    printf("Received the kill signal thing\n");

    // TODO: Find a way to kill the accept()

    shutdown(sockfd, SHUT_RD);

    char buffer[32];
    while (recv(sockfd, buffer, sizeof(buffer), 0) > 0) {}

    should_shutdown = true;
}

void Server::handle_client(socket_t clifd) {
}

bool Server::serve(int port) {
    should_shutdown = false;
    num_connections = 0;

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
        // TODO timeout if a client is stuck
    }

    stop_server(sockfd);

    return true;
}
