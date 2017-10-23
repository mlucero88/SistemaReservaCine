#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <sys/ipc.h>
#include <unordered_map>

#include "../common/constantes.h"
#include "../common/color_print.h"
#include "../common/ipc/sock.h"
#include "../common/ipc/msg_queue.h"

#define MOM_LOG(fmt, ...) FPRINTF(stdout, KGRN, fmt, ##__VA_ARGS__)

#define ARG_IFNAME		1
#define ARG_SERV_ADDR	2
#define ARG_PORT_ADDR	3
#define N_ARGS			4

#ifndef IFHWADDRLEN
#define IFHWADDRLEN 6
#endif

struct cli_data {
	pid_t cli_pid;
	pid_t socket_adapter_pid;
};

static std::unordered_map< uuid_t, cli_data > clientes;

/* Para debuguear. Ojo con el endianness al interpretar los hexas impresos en el for
void imprimir_uuid(uuid_t id) {
	printf("*******\n");
	printf("uuid = %li\n", id);
	uint8_t *p = reinterpret_cast<uint8_t*>(&id);
	for (int i = 0; i < 8; i++) {
		printf("%02X ", static_cast<unsigned int>(p[i]));
	}
	printf("\n*******\n");
}
*/

void salir() {
	for(const auto& cliPair : clientes) {
		MOM_LOG("Cerrando adapter %i...\n", cliPair.second.socket_adapter_pid);
		kill(cliPair.second.socket_adapter_pid, SIGUSR2);
	}

	MOM_LOG("Proceso finalizado\n");
	exit(0);
}

void handler(int signal) {
	salir();
}

int main(int argc, char *argv[]) {
	if (argc != N_ARGS) {
		MOM_LOG("Uso: %s <ifname> <serv_addr> <serv_port>\n", argv[0]);
		exit(1);
	}

	/* Señales */
	struct sigaction sigchld;
	sigchld.sa_handler = SIG_DFL;
	sigchld.sa_flags = SA_NOCLDWAIT;
	sigemptyset(&sigchld.sa_mask);
	sigaction(SIGCHLD, &sigchld, NULL);
	signal(SIGINT, handler);

	/* Apertura colas */
	int q_cli_snd = msg_queue_get(Q_MOM_CLI);
	int q_cli_rcv = msg_queue_get(Q_CLI_MOM);

	int q_adapter_snd = msg_queue_get(Q_CLI_CINE_A);
	int q_adapter_rcv = msg_queue_get(Q_CINE_CLI_A);

	if (q_cli_snd == -1 || q_cli_rcv == -1) {
		MOM_LOG("Error al crear canal de comunicacion entre mom y cliente\n");
		salir();
	}
	if (q_adapter_snd == -1 || q_adapter_rcv == -1) {
		MOM_LOG("Error al crear canal de comunicacion entre mom y socket_adapter\n");
		salir();
	}

	/* Obtencion mac address para uuid */
	unsigned char hw_addr_s[IFHWADDRLEN];
	if (sock_get_local_hwaddr(argv[ARG_IFNAME], hw_addr_s) > 0) {
		MOM_LOG("Error al obtener la mac address\n");
		salir();
	}

	MOM_LOG("MAC ADDRESS = ");
	int i;
	for (i = 0; i < IFHWADDRLEN - 1; i++) {
		MOM_LOG("%02X:", (unsigned int )(hw_addr_s[i]));
	}
	MOM_LOG("%02X\n", (unsigned int )(hw_addr_s[i]));

	uuid_t hw_addr = 0;
	for (int i = 0; i < IFHWADDRLEN; i++) {
		hw_addr = (hw_addr << 8) | hw_addr_s[i];
	}

	/* Loop procesamiento */
	mensaje_t msg;
	while (true) {
		if (msg_queue_receive(q_cli_rcv, 0, &msg)) {
			MOM_LOG("Recibo mensaje %s del cliente %li\n", strOpType(msg.tipo), msg.mtype);

			switch(msg.tipo) {
			case MOM_INIT: {
				pid_t cli_pid = msg.op.mom_init.cli_pid;
				uuid_t cli_id = cli_pid;
				cli_id = (cli_id << 48) | hw_addr;

				msg.mtype = cli_pid;
				msg.tipo = MOM_INIT_REPLY;

				pid_t adapter_pid = fork();
				if (adapter_pid == -1) {
					MOM_LOG("Falló el fork\n");
					msg.op.mom_init_reply.cli_id = -1;
				}
				else if (adapter_pid == 0) {
					execl("./socket_adapter_cliente", "socket_adapter_cliente", std::to_string(cli_id).c_str(), argv[ARG_SERV_ADDR], argv[ARG_PORT_ADDR], NULL);
					perror("Error al crear al proceso socket_adapter_cine\n");
					exit(1);
				}
				else {
					clientes.insert(std::make_pair(cli_id, cli_data { cli_pid, adapter_pid }));
					msg.op.mom_init_reply.cli_id = cli_id;
				}

				MOM_LOG("Envío respuesta %s al cliente %i\n", strOpType(msg.tipo), cli_pid);
				msg_queue_send(q_cli_snd, &msg);
				break;
			}
			case MOM_DESTROY: {
				auto it = clientes.find(msg.op.mom_destroy.cli_id);
				if(it != clientes.cend()) {
					kill(it->second.socket_adapter_pid, SIGUSR2);
					clientes.erase(msg.op.mom_destroy.cli_id);
				}
				break;
			}
			default: {
				/* Cuando hay timeout y nos avisa el cine (lo leo en el siguiente receive), este send deja en la cola un mensaje q el cine no lo lee xq se cerró */
				msg_queue_send(q_adapter_snd, &msg);
				msg_queue_receive(q_adapter_rcv, 0, &msg);

				/* Parche para arreglar esto */
				if (msg.tipo == TIMEOUT) {
					mensaje_t dummy;
					msg_queue_receive(q_adapter_snd, msg.mtype, &dummy, IPC_NOWAIT);
				}
				/* Fin parche */

				MOM_LOG("Envío respuesta %s al cliente %li\n", strOpType(msg.tipo), msg.mtype);
				msg_queue_send(q_cli_snd, &msg);
				break;
			}
			}
		}
	}

	return 0;
}
