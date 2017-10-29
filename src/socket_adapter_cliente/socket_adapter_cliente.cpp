#include <csignal>
#include <cstdlib>
#include <string>
#include <cstring>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"
#include "../common/ipc/sock.h"

#define ARG_CLI_ID		1
#define ARG_SERV_ADDR	2
#define ARG_PORT_ADDR	3
#define N_ARGS			4

#define SOCK_CLI_LOG(fmt, ...) FPRINTF(stdout, KCYN, "[ADAPTER_%li] " fmt, cli_id , ##__VA_ARGS__)
#define SOCK_CLI_SEND_LOG(fmt, ...) FPRINTF(stdout, KCYN, "[ADAPTER_SEND_%li] " fmt, cli_id , ##__VA_ARGS__)
#define SOCK_CLI_RCV_LOG(fmt, ...) FPRINTF(stdout, KCYN, "[ADAPTER_RECV_%li] " fmt, cli_id , ##__VA_ARGS__)

static int sock_id = -1;
static uuid_t cli_id = -1;
static pid_t send_pid = -1;

void salir() {
	if (send_pid > 0) {
		kill(send_pid, SIGTERM);
	}

	if (sock_id > 0) {
		close(sock_id);
	}
	SOCK_CLI_LOG("Proceso finalizado\n");
    exit(0);
}

void handler(int signal) {
    salir();
}

int main(int argc, char *argv[]) {
    if (argc != N_ARGS) {
    	SOCK_CLI_LOG("Uso: %s <cli_id> <serv_addr> <serv_port>\n", argv[0]);
        exit(1);
    }

    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);
	signal(SIGINT, handler);	// Al hacer fork y no hacer exec, los hijos heredan el registro del handler

    char *serv_addr = argv[ARG_SERV_ADDR];
    uint16_t serv_port = std::stoi(argv[ARG_PORT_ADDR]);
    cli_id = std::atol(argv[ARG_CLI_ID]);

    SOCK_CLI_LOG("Atiendo cliente %li\n", cli_id);

    int q_mom_snd = msg_queue_get(Q_CINE_CLI_A);
    int q_mom_rcv = msg_queue_get(Q_CLI_CINE_A);
    int q_admin_rcv = msg_queue_get(Q_ADMIN_CLI_A);
    if (q_mom_snd == -1 || q_mom_rcv == -1 || q_admin_rcv == -1) {
    	SOCK_CLI_LOG("Error al crear canal de comunicacion entre socket_adapter y mom\n");
        salir();
    }

    sock_id = sock_create();
    if (sock_id == -1) {
    	SOCK_CLI_LOG("Error al crear el socket del cliente\n");
        salir();
    }
    int r = sock_connect(sock_id, serv_addr, serv_port);
    if (r != 0) {
    	SOCK_CLI_LOG("No se pudo conectar al servidor\n");
        salir();
    }
    SOCK_CLI_LOG("Conexion a %s:%hu establecida\n", serv_addr, serv_port);

    send_pid = fork();

    if (send_pid == -1) {
    	SOCK_CLI_LOG("Error en fork!\n");
    	salir();
    }

    mensaje_t msg;
    if (send_pid == 0) {
        // Proceso que envía
    	signal(SIGINT, SIG_IGN);	// Para no cerrarlo con el CTRL+C. Lo cierra el padre

        while (true) {
            msg_queue_receive(q_mom_rcv, cli_id, &msg);
            SOCK_CLI_SEND_LOG("Recibí pedido %s del cliente\n", strOpType(msg.tipo));

            /* Esto tengo que ponerlo aca, aunque esta logica no deberia pertenecer al adapter.
             * Pero si me viniese el mtype=LOGIN_MSG_TYPE desde el mom, el adapter no lo recibiria nunca */
            if (msg.tipo == LOGIN) {
                msg.mtype = LOGIN_MSG_TYPE;
            }

            if (sock_send(sock_id, &msg) != sizeof(msg)) {
            	SOCK_CLI_SEND_LOG("Error al enviar mensaje al cine - %s\n", std::strerror(errno));
                continue;
            }
            SOCK_CLI_SEND_LOG("Envié pedido %s al cine\n", strOpType(msg.tipo));
        }

    } else {
        // Proceso que recibe
        while (true) {
            int bytesRec = sock_recv(sock_id, &msg);
            if (bytesRec == 0) {
            	// Desconexion del peer. El cine no deberia iniciar un cierre
            	SOCK_CLI_RCV_LOG("Socket remoto cerrado (no deberia entrar aca)\n");
                salir();
            } else if (bytesRec != sizeof(msg)) {
            	SOCK_CLI_RCV_LOG("Error al recibir mensaje del cine - %s\n", std::strerror(errno));
                continue;
            }

            if (msg.tipo == NOTIFICAR_CAMBIOS) {
                // Si es una notificacion del admin, mandar el mensaje a la cola de notificaciones
            	SOCK_CLI_RCV_LOG("Recibí notificación de asientos en sala %i\n", msg.op.info_asientos.nro_sala);
                msg_queue_send(q_admin_rcv, &msg);
                SOCK_CLI_RCV_LOG("Envié notificación al cliente\n");
            } else {
            	// Si es una respuesta del cine, mandar a la cola común
            	SOCK_CLI_RCV_LOG("Recibí respuesta %s del cine\n", strOpType(msg.tipo));
                msg_queue_send(q_mom_snd, &msg);
                SOCK_CLI_RCV_LOG("Envié respuesta %s al cliente\n", strOpType(msg.tipo));
            }
        }
    }
}
