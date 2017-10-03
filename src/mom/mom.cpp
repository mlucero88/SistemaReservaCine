#include <csignal>
#include <cstdlib>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"

#define MOM_LOG(fmt, ...) FPRINTF(stdout, KWHT, fmt, ##__VA_ARGS__)

volatile bool quit = false;

void salir() {
	MOM_LOG("Proceso finalizado\n");
	exit(0);
}

void handler(int signal) {
	quit = true;
}

int main(int argc, char *argv[]) {
	signal(SIGINT, handler);

	int q_cli_snd = msg_queue_get(Q_MOM_CLI);
	int q_cli_rcv = msg_queue_get(Q_CLI_MOM);

	int q_cine_snd = msg_queue_get(Q_CLI_CINE);
	int q_cine_rcv = msg_queue_get(Q_CINE_CLI);

	if (q_cli_snd == -1 || q_cli_rcv == -1) {
		MOM_LOG("Error al crear canal de comunicacion entre mom y cliente\n");
		salir();
	}
	if (q_cine_snd == -1 || q_cine_rcv == -1) {
		MOM_LOG("Error al crear canal de comunicacion entre cliente y cine\n");
		salir();
	}

	mensaje_t msg;
	while (!quit) {
        msg_queue_receive(q_cli_rcv, 0, &msg);
        MOM_LOG("Recibo mensaje %s del cliente %i y lo mando al cine\n", strOpType(msg.tipo), msg.mtype);
        msg_queue_send(q_cine_snd, &msg);
        msg_queue_receive(q_cine_rcv, 0, &msg);
        MOM_LOG("Envío respuesta %s al cliente %i\n", strOpType(msg.tipo), msg.mtype);
        msg_queue_send(q_cli_snd, &msg);
	}

	salir();
}
