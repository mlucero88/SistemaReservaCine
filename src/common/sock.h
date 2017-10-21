#ifndef SISTEMARESERVACINE_SOCK_H
#define SISTEMARESERVACINE_SOCK_H

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

int sock_create() {
    int sock_id = socket(AF_INET, SOCK_STREAM, 0);
    return sock_id;
}

int sock_connect(int sock_id, const char *addr, int port) {
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(addr);
    sock_addr.sin_port = htons(port);
    socklen_t sock_len;
    return connect(sock_id, (struct sockaddr *) &sock_addr, sock_len);
}

int sock_bind(int sock_id, int port) {
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(port);
    bind(sock_id, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
}

int sock_listen(int sock_id, int n) {
    listen(sock_id, n);
}

int sock_accept(int sock_id) {
//    struct sockaddr_in sock_addr;
//    socklen_t sock_len;
//    return accept(sock_id, (struct sockaddr *) &sock_addr, &sock_len);
    return accept(sock_id, NULL, NULL);
}

int sock_send(int sock_id, const void *buf, size_t n) {
    return write(sock_id, buf, n);
}

int sock_recv(int sock_id, void *buf, size_t n) {
    int total = 0;
    while (total < n) {
        int r = read(sock_id, (char *) buf + total, n - total);
        if (r > 0) {
            total += r;
        } else {
            // Error
            return r;
        }
    }
    return total;
}

#endif //SISTEMARESERVACINE_SOCK_H
