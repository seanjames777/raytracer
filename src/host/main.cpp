/**
 * @file main.cpp
 *
 * @brief Host executable entrypoint
 *
 * @author Sean James
 */

#include <host/glimagedisplay.h>
#include <string.h>
#include <net/protocol.h>

int main(int argc, char *argv[]) {
    RTProtocolConnection conn;

	if (!conn.connect_to_server("localhost", 7878)) {
		printf("Connect to server failed\n");
		exit(-1);
	}

    SERVER_STATUS stat = WAITING;

    std::shared_ptr<Image> image = std::make_shared<Image>(1920, 1080);

    GLImageDisplay *disp = new GLImageDisplay(1920, 1080, image);

    conn.beginRender();

    while (stat != FINISHED) {
        conn.getImage(image);
		std::this_thread::sleep_for(std::chrono::milliseconds(80));

        conn.getStatus(&stat);
    }

    conn.getImage(image);

    printf("Press enter to quit...\n");
    getchar();

    conn.shutdown();

    delete disp;

    return 0;
}
