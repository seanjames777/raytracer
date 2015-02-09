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

bool display = false;

void parseArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-display") == 0)
            display = true;
    }
}

int main(int argc, char *argv[]) {
    parseArgs(argc, argv);

    RTProtocolConnection conn;

	if (!conn.connect_to_server("localhost", 7878)) {
		printf("Connect to server failed\n");
		exit(-1);
	}

    SERVER_STATUS stat = WAITING;

    std::shared_ptr<Image> image = std::make_shared<Image>(1920, 1080);

    std::shared_ptr<GLImageDisplay> disp = nullptr;

    if (display)
        disp = std::make_shared<GLImageDisplay>(1920, 1080, image);

    conn.beginRender();

    while (stat != FINISHED) {
        conn.getImage(image);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        conn.getStatus(&stat);
    }

    conn.getImage(image);

    conn.shutdown();

    printf("Press enter to quit...\n");
    getchar();

    return 0;
}
