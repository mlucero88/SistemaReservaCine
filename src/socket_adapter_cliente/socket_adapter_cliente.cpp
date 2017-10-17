#include <csignal>
#include <cstdlib>

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
    int q_cine_snd = msg_queue_get(Q_CLI_CINE);
    int q_cine_rcv = msg_queue_get(Q_CINE_CLI);

    if (q_cine_snd == -1 || q_cine_rcv == -1) {
        MOM_LOG("Error al crear canal de comunicacion entre cliente y cine\n");
        salir();
    }

    int sock_id = sock_create();
    int r = sock_connect(sock_id, SERV_ADDR, SERV_PORT);
    if (r != 0) {
        MOM_LOG("No se pudo conectar al servidor\n");
        salir();
    }

    mensaje_t msg;
    while (true) {
        msg_queue_receive(q_cine_snd, 0, &msg);// Recive del mom
        r = sock_send(sock_id, &msg, sizeof(msg)); // Envía por el socket al cine
        if (r !=) {
            MOM_LOG("No se pudo enviar mensaje al servidor\n");
            salir();
        }
        sock_recv(sock_id, &msg, sizeof(msg)); // Recibe respuesta del cine
        if (r !=) {
            MOM_LOG("No se pudo recibir respuesta del servidor\n");
            salir();
        }
        msg_queue_send(q_cine_rcv, &msg); // Envía respuesta al mom
    }

    return 0;
}
