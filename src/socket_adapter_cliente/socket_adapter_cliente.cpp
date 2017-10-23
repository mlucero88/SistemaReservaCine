#include <csignal>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cerrno>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"
#include "../common/ipc/sock.h"

#define ARG_CLI_ID		1
#define ARG_SERV_ADDR	2
#define ARG_PORT_ADDR	3
#define N_ARGS			4

#define SOCK_CLI_LOG(fmt, ...) FPRINTF(stdout, KCYN, "[ADAPTER_%i] " fmt, getpid() , ##__VA_ARGS__)

#define _DEBUG
#ifdef _DEBUG
#define SOCK_CLI_LOG_DEBUG(fmt, ...) FPRINTF(stdout, KCYN, "[ADAPTER_%i] " fmt, getpid() , ##__VA_ARGS__)
#else
#define SOCK_CLI_LOG_DEBUG(fmt, ...)
#endif

static int sock_id = -1;

void salir() {
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
	signal(SIGUSR2, handler);

    char *serv_addr = argv[ARG_SERV_ADDR];
    uint16_t serv_port = std::stoi(argv[ARG_PORT_ADDR]);
    const uuid_t cli_id = std::atol(argv[ARG_CLI_ID]);

    SOCK_CLI_LOG("Atiendo cliente %li\n", cli_id);

    int q_mom_snd = msg_queue_get(Q_CINE_CLI_A);
    int q_mom_rcv = msg_queue_get(Q_CLI_CINE_A);

    if (q_mom_snd == -1 || q_mom_rcv == -1) {
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

    mensaje_t msg;
    while (true) {
        msg_queue_receive(q_mom_rcv, cli_id, &msg);
        SOCK_CLI_LOG_DEBUG("Recibí pedido %s del cliente\n", strOpType(msg.tipo));

        /* Esto tengo que ponerlo aca, aunque esta logica no deberia pertenecer al adapter.
         * Pero si me viniese el mtype=LOGIN_MSG_TYPE desde el mom, el adapter no lo recibiria nunca */
        if(msg.tipo == LOGIN) {
        	msg.mtype = LOGIN_MSG_TYPE;
        }

        if (sock_send(sock_id, &msg) != sizeof(msg)) {
        	SOCK_CLI_LOG("Error al enviar mensaje al cine - %s\n", std::strerror(errno));
//          salir();
        	continue;
        }
        SOCK_CLI_LOG_DEBUG("Envié pedido %s al cine\n", strOpType(msg.tipo));

        int bytesRec =  sock_recv(sock_id, &msg);
        if (bytesRec == 0) {
        	// Desconexion del peer
            salir();
        }
        else if (bytesRec != sizeof(msg)) {
        	SOCK_CLI_LOG("Error al recibir mensaje del cine - %s\n", std::strerror(errno));
//          salir();
        	continue;
        }
        SOCK_CLI_LOG_DEBUG("Recibí respuesta %s del cine\n", strOpType(msg.tipo));

        msg_queue_send(q_mom_snd, &msg);
        SOCK_CLI_LOG_DEBUG("Envié respuesta %s al cliente\n", strOpType(msg.tipo));
    }

    return 0;
}
