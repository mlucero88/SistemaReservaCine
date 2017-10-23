#ifndef SISTEMARESERVACINE_SOCK_H
#define SISTEMARESERVACINE_SOCK_H

#include <cstdint>

#include "msg_queue.h"

int sock_create();

int sock_connect(int sock_id, const char *addr, uint16_t port);

int sock_bind(int sock_id, uint16_t port);
int sock_listen(int sock_id, int n);
int sock_accept(int sock_id);

int sock_send(int sock_id, const mensaje_t* msg);
int sock_recv(int sock_id, mensaje_t* msg);

int sock_get_local_hwaddr(const char *ifname, unsigned char *mac);

#endif //SISTEMARESERVACINE_SOCK_H
