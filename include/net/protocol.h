/**
 * @file protocol.h
 *
 * @brief Raytracing server network protocol interface
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <net/server.h>
#include <net/client.h>
#include <image.h>
#include <memory>

enum REQUEST_TYPE {
    BEGIN_RENDER = 0,
    UPDATE_IMAGE = 1,
    GET_STATUS = 2,
    SHUTDOWN = 3,
};

enum SERVER_STATUS {
    WAITING = 0,
    WORKING = 1,
    FINISHED = 2
};

#pragma pack(push, 1)

struct RequestHeader {
    int request_type;
};

#pragma pack(pop)

/**
 * @file The host coordinates the work of any connected servers, and provides a
 * user interface/preview to the user.
 */
class RTProtocolConnection : public Client {
private:

    bool sendRequestHeader(RequestHeader *header) {
		socket_t sockfd = getSocket();

        if (!write_buff(sockfd, (char *)header, sizeof(RequestHeader))) {
            printf("Error writing request header\n");
            return false;
        }

        return true;
    }

public:

    bool getStatus(SERVER_STATUS *status) {
		socket_t sockfd = getSocket();

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

    bool updateImage(std::shared_ptr<Image> image) {
		socket_t sockfd = getSocket();

        RequestHeader req;
        req.request_type = UPDATE_IMAGE;

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

		disconnect_from_server();

        return true;
    }

};

/**
 * @file The worker does the actual work of raytracing and acts as a server.
 */
class RTProtocolServer : public Server {
protected:

    virtual SERVER_STATUS handleGetStatus() {
        return WAITING;
    }

    virtual void handleBeginRender() {
    }

    virtual void handleShutdown() {
    }

    virtual std::shared_ptr<Image> getImage() = 0;

    bool writeImage(socket_t fd, std::shared_ptr<Image> image) {
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

    RTProtocolServer() {
    }

    ~RTProtocolServer() {
    }

	virtual void handle_client(socket_t clifd) override {
		std::shared_ptr<Image> image;
		SERVER_STATUS stat;

		// TODO: send an acknowledgement?
		// TODO: shutdown vs disconnect

		while (true) {
			RequestHeader header;

			if (!read_buff(clifd, (char *)&header, sizeof(header))) {
				printf("Error reading request header\n");
                handleShutdown();
				return;
			}

			switch (header.request_type) {
			case BEGIN_RENDER:
				handleBeginRender();
				break;
			case UPDATE_IMAGE:
				image = getImage();

				if (!writeImage(clifd, image))
					return;
				break;
			case GET_STATUS:
                stat = handleGetStatus();

				if (!write_buff(clifd, (char *)&stat, sizeof(stat))) {
					printf("Error writing response\n");
					return;
				}

				break;
			case SHUTDOWN:
                handleShutdown();
				setShouldShutDown();
				return;
			default:
				handleShutdown();
				return;
			}
		}
	}

};

#endif
