/**
 * @file protocol.h
 *
 * @brief Raytracing server network protocol interface
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <memory>
#include <image.h>

enum REQUEST_TYPE {
    BEGIN_RENDER = 0,
    GET_IMAGE = 1,
    GET_STATUS = 2,
    SHUTDOWN = 3,
};

enum SERVER_STATUS {
    WAITING = 0,
    WORKING = 1,
    FINISHED = 2
};

struct RequestHeader {
    enum REQUEST_TYPE request_type;
};

#define BUFF_SIZE 4096

bool write_buff(int fd, char *buff, int len) {
    while (len > 0) {
        int write_count = write(fd, buff, len < BUFF_SIZE ? len : BUFF_SIZE);

        if (write_count < 0)
            return false;

        buff += write_count;
        len -= write_count;
    }

    return true;
}

bool read_buff(int fd, char *buff, int len) {
    while (len > 0) {
        int read_count = read(fd, buff, len < BUFF_SIZE ? len : BUFF_SIZE);

        if (read_count < 0)
            return false;

        buff += read_count;
        len -= read_count;
    }

    return true;
}

/**
 * @file The host coordinates the work of any connected servers, and provides a
 * user interface/preview to the user.
 */
class Connection {
private:

    int sockfd = -1;

    bool sendRequestHeader(RequestHeader *header) {
        if (!write_buff(sockfd, (char *)header, sizeof(RequestHeader))) {
            printf("Error writing request header\n");
            return false;
        }

        return true;
    }

public:

    bool connectToWorker(std::string addr, int port) {
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
                close (sockfd);
                usleep(1000 * 1000);
                continue;
            }

            connected = true;
            break;
        }

        if (!connected) {
            printf("Error connecting to worker: %s:%d\n", addr.c_str(), port);
            return false;
        }

        printf("Connected to worker: %s:%d\n", addr.c_str(), port);

        return true;
    }

    bool getStatus(SERVER_STATUS *status) {
        RequestHeader req;
        req.request_type = GET_STATUS;

        if (!sendRequestHeader(&req)) {
            printf("Error requesting server status\n");
            return false;
        }

        if (!read_buff(sockfd, (char *)status, sizeof(SERVER_STATUS))) {
            printf("Error reading server status\n");
            return false;
        }

        return true;
    }

    bool beginRender() {
        RequestHeader req;
        req.request_type = BEGIN_RENDER;

        if (!sendRequestHeader(&req)) {
            printf("Error sending begin render message\n");
            return false;
        }

        return true;
    }

    bool getImage(std::shared_ptr<Image> image) {
        RequestHeader req;
        req.request_type = GET_IMAGE;

        if (!sendRequestHeader(&req)) {
            printf("Error requesting image\n");
            return false;
        }

        int width = image->getWidth();
        int height = image->getHeight();
        int size = width * height * 4 * sizeof(float);

        float *pixels = (float *)malloc(size);

        if (!read_buff(sockfd, (char *)pixels, size)) {
            printf("Error reading image\n");
            return false;
        }

        image->setPixels(pixels);

        free(pixels);

        return true;
    }

    bool shutdown() {
        RequestHeader req;
        req.request_type = SHUTDOWN;

        if (!sendRequestHeader(&req)) {
            printf("Error shutting down server\n");
            return false;
        }

        printf("Host shutdown\n");
        close(sockfd);

        return true;
    }

};

/**
 * @file The worker does the actual work of raytracing and acts as a server.
 */
class Server {
protected:

    virtual SERVER_STATUS handleGetStatus() {
        return WAITING;
    }

    virtual void handleBeginRender() {
    }

    virtual void handleShutdown() {
    }

    virtual std::shared_ptr<Image> getImage() = 0;

    bool writeImage(int fd, std::shared_ptr<Image> image) {
        int width = image->getWidth();
        int height = image->getHeight();
        int size = width * height * 4 * sizeof(float);

        float *pixels = (float *)malloc(size);
        image->getPixels(pixels);

        if (!write_buff(fd, (char *)pixels, size)) {
            printf("Error writing image\n");
            return false;
        }

        free(pixels);

        return true;
    }

public:

    Server() {
    }

    ~Server() {
    }

    bool serve(int port) {
        // TODO: Clean various things up when returning an error

        int sockfd;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Error creating socket\n");
            return false;
        }

        int opt = 1;

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
            printf("Error setting SO_REUSEADDR\n");
            return false;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("Error binding socket\n");
            return false;
        }

        listen(sockfd, 32);

        printf("Worker: listening on port %d\n", port);

        socklen_t clilen;
        struct sockaddr_in cli_addr;
        int clientfd;

        clilen = sizeof(cli_addr);

        // TODO: for now, only accept one connection

        if ((clientfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0) {
            printf("Error accepting connection\n");
            return false;
        }

        printf("Connected to %s:%d\n",
            inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        bool shutdown = false;
        std::shared_ptr<Image> image;

        while (!shutdown) {
            RequestHeader header;
            if (!read_buff(clientfd, (char *)&header, sizeof(header))) {
                printf("Error reading request header\n");
                return false;
            }

            SERVER_STATUS stat;

            switch(header.request_type) {
            case BEGIN_RENDER:
                handleBeginRender();

                break;
            case GET_IMAGE:
                image = getImage();

                if (!writeImage(clientfd, image))
                    return false;

                break;
            case GET_STATUS:
                stat = handleGetStatus();

                if (!write_buff(clientfd, (char *)&stat, sizeof(stat))) {
                    printf("Error writing response\n");
                    return false;
                }

                break;
            case SHUTDOWN:
                handleShutdown();

                // TODO: send an acknowledgement?
                shutdown = true;
                break;
            }
        }

        close(clientfd);

        close(sockfd);

        printf("Server shutdown\n");

        return true;
    }

};

#endif
