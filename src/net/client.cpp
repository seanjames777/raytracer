/**
* @file client.cpp
*
* @author Sean James <seanjames777@gmail.com>
*/

#include <net/client.h>

Client::Client() {
}

Client::~Client() {
}

bool Client::connect_client(std::string host, int port, socket_t *sockfd_out, int max_attempts) {
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

    for (int i = 0; i < max_attempts; i++) {
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

        if (sockfd == INVALID_SOCKET)
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        else
            break;
    }

    freeaddrinfo(result);

    if (sockfd == INVALID_SOCKET) {
        printf("Unable to connect to server\n");
        WSACleanup();
        return false;
    }
#else
    // TODO: comment

    int sockfd;
    struct hostent *server;
    server = gethostbyname(host.c_str());

    if (server == NULL) {
        printf("Error parsing address string: %s\n", host.c_str());
        return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    bool connected = false;
    int attempts = 0;

    while (!connected && attempts++ < max_attempts) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Error creating socket\n");
            return false;
        }

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sockfd);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        connected = true;
        break;
    }

    if (!connected) {
        printf("Error connecting to worker: %s:%d\n", host.c_str(), port);
        close(sockfd);
        return false;
    }
#endif

    printf("Connected to worker: %s:%d\n", host.c_str(), port);

    *sockfd_out = sockfd;

    return true;
}

void Client::stop_client(socket_t sockfd) {
#ifdef WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
}

socket_t Client::getSocket() {
    return sockfd;
}

bool Client::connect_to_server(std::string host, int port, int max_attempts) {
    return connect_client(host, port, &sockfd, max_attempts);
}

void Client::disconnect_from_server() {
    stop_client(sockfd);
}
