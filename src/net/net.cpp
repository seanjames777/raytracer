/**
 * @file net.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <net/net.h>

bool write_buff(socket_t fd, char *buff, int len) {
    while (len > 0) {
        int write_count = send(fd, buff, len < BUFF_SIZE ? len : BUFF_SIZE, 0);

        if (write_count < 0)
            return false;

        buff += write_count;
        len -= write_count;
    }

    return true;
}

bool read_buff(socket_t fd, char *buff, int len) {
    while (len > 0) {
        int read_count = recv(fd, buff, len < BUFF_SIZE ? len : BUFF_SIZE, 0);

        if (read_count < 0)
            return false;

        buff += read_count;
        len -= read_count;
    }

    return true;
}
