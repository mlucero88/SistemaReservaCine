#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <sys/ipc.h>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"

#define CINE_LOG(fmt, ...) FPRINTF(stdout, KRED, "[CINE_%i] " fmt, getpid() , ##__VA_ARGS__)

static int cli_id;
static int n_sala;

static int q_cli_snd;
static int q_cli_rcv;

static int q_admin_snd;
static int q_admin_rcv;

void salir() {
	CINE_LOG("Proceso finalizado\n");
	exit(0);
}

void alarm_handler(int signal) {
	CINE_LOG("Session time expired for client %i\n", cli_id);
	mensaje_t msg, dummyMsg;
	msg.mtype = cli_id;
	msg.tipo = TIMEOUT;
	msg.op.timeout.cli_id = cli_id;
	msg.op.timeout.n_sala = n_sala;

	msg_queue_send(q_admin_snd, &msg);
	msg_queue_send(q_cli_snd, &msg);

	/* rcv no bloqueante para sacar algun mensaje q el cliente pudo haber mandado */
	msg_queue_receive(q_cli_rcv, cli_id, &dummyMsg, IPC_NOWAIT);

	salir();
}

void forward_msg(int tipo) {
	mensaje_t msg;

	if (msg_queue_receive(q_cli_rcv, cli_id, &msg)) {
		if (msg.tipo == ELEGIR_SALA) {
			n_sala = msg.op.elegir_sala.nro_sala;
		}
		if (msg_queue_send(q_admin_snd, &msg)) {
			CINE_LOG("Esperando respuesta del admin para %s\n", strOpType(tipo));
			if (msg_queue_receive(q_admin_rcv, cli_id, &msg)) {
				if (msg_queue_send(q_cli_snd, &msg)) {
					if (msg.tipo == RESERVA_CANCELADA) {
						salir();
					}
					return;
				}
			}
		}
	}

	CINE_LOG("Hubo un error en la operacion %s\n", strOpType(tipo));

	/* Tengo que avisarle al admin para que lo quite del sistema. Aprovecho el uso de timeout */
	msg.mtype = cli_id;
	msg.tipo = TIMEOUT;
	msg.op.timeout.cli_id = cli_id;
	msg.op.timeout.n_sala = n_sala;
	msg_queue_send(q_admin_snd, &msg);

	salir();
}

int main(int argc, char *argv[]) {
	struct sigaction sigalarm;
	sigalarm.sa_handler = alarm_handler;
	sigemptyset(&sigalarm.sa_mask);
	sigaction(SIGALRM, &sigalarm, NULL);

	cli_id = atoi(argv[1]);
	CINE_LOG("Iniciado proceso para cliente [%i]\n", cli_id);

	q_cli_snd = msg_queue_get(Q_CINE_CLI_B);
	q_cli_rcv = msg_queue_get(Q_CLI_CINE_B);

	q_admin_snd = msg_queue_get(Q_CINE_ADMIN);
	q_admin_rcv = msg_queue_get(Q_ADMIN_CINE);

	if (q_admin_snd == -1 || q_admin_rcv == -1) {
		CINE_LOG("Error al crear canal de comunicacion entre cine y admin\n");
		salir();
	}
	if (q_cli_snd == -1 || q_cli_rcv == -1) {
		CINE_LOG("Error al crear canal de comunicacion entre cine y cliente\n");
		salir();
	}

	/* Empieza tiempo de sesion */
	alarm(TIMEOUT_VAL);

	mensaje_t msg = { .mtype = cli_id, .tipo = INFORMAR_SALAS };
	if (msg_queue_send(q_admin_snd, &msg)) {
		CINE_LOG("Esperando respuesta del admin para INFORMAR_SALAS\n");
		if (msg_queue_receive(q_admin_rcv, cli_id, &msg)) {
			msg_queue_send(q_cli_snd, &msg);
		}
	}

	forward_msg(ELEGIR_SALA);
	forward_msg(ELEGIR_ASIENTOS);
	forward_msg(CONFIRMAR_RESERVA);
	forward_msg(PAGAR);

	salir();
}
