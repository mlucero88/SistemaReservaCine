/*
 * sock.c
 *
 *  Created on: 21 oct. 2017
 *      Author: martin
 */

#include "sock.h"

#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

int sock_create() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

int sock_connect(int sock_id, const char *addr, uint16_t port) {
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(addr);
    sock_addr.sin_port = htons(port);
    return connect(sock_id, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
}

int sock_bind(int sock_id, uint16_t port) {
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(port);
    return bind(sock_id, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
}

int sock_listen(int sock_id, int n) {
    return listen(sock_id, n);
}

int sock_accept(int sock_id) {
    return accept(sock_id, NULL, NULL);
}

int sock_send(int sock_id, const mensaje_t* msg) {
	// todo pasar campos a network-endianness
    return send(sock_id, msg, sizeof(mensaje_t), 0);
}

int sock_recv(int sock_id, mensaje_t* msg) {
	// todo pasar campos a host-endianness
    int total = 0;
    const int n = sizeof(mensaje_t);

    while (total < n) {
        int r = recv(sock_id, reinterpret_cast<char*>(msg) + total, n - total, 0);
        if (r > 0) {
            total += r;
        } else {
            // Error ( <  0) o desconexion ( == 0)
            return r;
        }
    }
    return total;
}

int sock_get_local_hwaddr(const char *ifname, unsigned char *mac) {
	struct ifreq ifr;
	int fd;
	int ret;

	/* determine the local MAC address */
	strcpy(ifr.ifr_name, ifname);
	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0) {
		ret = fd;
	} else {
		ret = ioctl(fd, SIOCGIFHWADDR, &ifr);
		if (ret >= 0) {
			memcpy(mac, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
		}
	}

	return ret;
}
