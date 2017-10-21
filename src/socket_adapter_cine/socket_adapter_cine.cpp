#include <csignal>
#include <cstdlib>
#include <iostream>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"
#include "../common/sock.h"

#define MOM_LOG(fmt, ...) FPRINTF(stdout, KWHT, fmt, ##__VA_ARGS__)

void salir() {
    MOM_LOG("Proceso finalizado\n");
    exit(0);
}

void handler(int signal) {
    salir();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "socket_adaper_cine <serv port>" << std::endl;
        exit(0);
    }
    int serv_port = atoi(argv[2]);
    int q_cine_rcv = msg_queue_get(Q_CLI_CINE_B);
    int q_cine_snd = msg_queue_get(Q_CINE_CLI_B);

    if (q_cine_snd == -1 || q_cine_rcv == -1) {
        MOM_LOG("Error al crear canal de comunicacion entre cliente y cine\n");
        salir();
    }

    int sock_id = sock_create();
    int r = sock_bind(sock_id, serv_port);
    if (r != 0) {
        MOM_LOG("No se crear el socket para el servidor\n");
        salir();
    }
    r = sock_listen(sock_id, MAX_CLIENTES);
    if (r != 0) {
        MOM_LOG("No se crear el socket para el servidor\n");
        salir();
    }

    mensaje_t msg;
    while (true) {
        r = sock_accept(sock_id);
        if (r == -1) {
            MOM_LOG("Error al aceptar conexión\n");
            salir();
        }
        int new_sock_id = r;
        if (fork() == 0) {
            r = sock_recv(new_sock_id, &msg, sizeof(msg)); // Recibe mensaje del cliente
            if (r != sizeof(msg)) {
                MOM_LOG("No se pudo recibir mensaje del cliente\n");
                salir();
            }
            msg_queue_send(q_cine_rcv, &msg); // Envía mensaje al cine a Q_CLI_CINE
            msg_queue_receive(q_cine_snd, 0, &msg);// Recibe respuesta del cine en Q_CINE_CLI
            r = sock_send(new_sock_id, &msg, sizeof(msg)); // Envía respuesta al cliente
            if (r != sizeof(msg)) {
                MOM_LOG("No se pudo enviar respuesta al cliente\n");
                salir();
            }
        }
    }
}
