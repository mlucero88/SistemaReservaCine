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
    if (argc < 3) {
        std::cout << "socket_adapter_cliente <serv addr> <serv port>" << std::endl;
        exit(0);
    }
    char *serv_addr = argv[1];
    int serv_port = atoi(argv[2]);

    int q_cine_snd = msg_queue_get(Q_CLI_CINE_A);
    int q_cine_rcv = msg_queue_get(Q_CINE_CLI_A);

    if (q_cine_snd == -1 || q_cine_rcv == -1) {
        MOM_LOG("Error al crear canal de comunicacion entre cliente y cine\n");
        salir();
    }

    int sock_id = sock_create();
    int r = sock_connect(sock_id, serv_addr, serv_port);
    if (r != 0) {
        MOM_LOG("No se pudo conectar al servidor\n");
        salir();
    }

    mensaje_t msg;
    while (true) {
        msg_queue_receive(q_cine_snd, 0, &msg);// Recive del mom en Q_CLI_CINE
        r = sock_send(sock_id, &msg, sizeof(msg)); // Envía por el socket al cine
        if (r != sizeof(msg)) {
            MOM_LOG("No se pudo enviar mensaje al servidor\n");
            salir();
        }
        sock_recv(sock_id, &msg, sizeof(msg)); // Recibe respuesta del cine
        if (r != sizeof(msg)) {
            MOM_LOG("No se pudo recibir respuesta del servidor\n");
            salir();
        }
        msg_queue_send(q_cine_rcv, &msg); // Envía respuesta al mom a Q_CINE_CLI
    }

    return 0;
}
