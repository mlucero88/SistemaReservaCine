#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"
#include "../common/ipc/sock.h"

#define ARG_PORT_ADDR	1
#define N_ARGS			2

#define SOCK_CINE_LOG(fmt, ...) FPRINTF(stdout, KCYN, fmt, ##__VA_ARGS__)
#define SOCK_CINE_HIJO_LOG(fmt, ...) FPRINTF(stdout, KMAG, "[ADAPTER_%i] " fmt, getpid() , ##__VA_ARGS__)

#define _DEBUG
#ifdef _DEBUG
#define SOCK_CINE_HIJO_LOG_DEBUG(fmt, ...) FPRINTF(stdout, KMAG, "[ADAPTER_%i] " fmt, getpid() , ##__VA_ARGS__)
#else
#define SOCK_CINE_HIJO_LOG_DEBUG(fmt, ...)
#endif

static int sock_id = -1;

void salir() {
	if (sock_id > 0) {
		close(sock_id);
	}
	SOCK_CINE_LOG("Proceso finalizado\n");
    exit(0);
}

void handler(int signal) {
    salir();
}

int main(int argc, char *argv[]) {
    if (argc < N_ARGS) {
    	SOCK_CINE_LOG("Uso: %s <serv_port>\n", argv[0]);
        exit(1);
    }

    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);
    signal(SIGUSR2, handler);

    uint16_t serv_port = std::atoi(argv[ARG_PORT_ADDR]);
    int q_cine_snd = msg_queue_get(Q_CLI_CINE_B);
    int q_cine_rcv = msg_queue_get(Q_CINE_CLI_B);

    if (q_cine_snd == -1 || q_cine_rcv == -1) {
    	SOCK_CINE_LOG("Error al crear canal de comunicacion entre socket_adapter y cine\n");
        salir();
    }

    sock_id = sock_create();
    if (sock_id == -1 || sock_bind(sock_id, serv_port) != 0 || sock_listen(sock_id, MAX_CLIENTES) != 0) {
    	SOCK_CINE_LOG("Error al crear el socket servidor del cine\n");
        salir();
    }

    while (true) {
        int new_sock_id = sock_accept(sock_id);
        if (new_sock_id == -1) {
        	SOCK_CINE_LOG("Error al aceptar conexión\n");
        	continue;
        }

        pid_t connection_pid = fork();
        if (connection_pid == -1) {
        	SOCK_CINE_LOG("Falló el fork del hijo socket_adapter\n");
        }
        if (connection_pid == 0) {
        	close(sock_id);

            mensaje_t msg;
            uuid_t cli_id = -1;

            // Se cierra cuando detecta desconexion del peer
        	while (true) {
        		int bytesRec = sock_recv(new_sock_id, &msg); // Recibe mensaje del cliente
        		if (bytesRec == 0) {
        			// Desconexion del peer
        			close(new_sock_id);
        			SOCK_CINE_HIJO_LOG("Proceso finalizado\n");
        			exit(0);
        		}
        		else if (bytesRec != sizeof(msg)) {
        			SOCK_CINE_HIJO_LOG("Error al recibir mensaje del cliente - %s\n", std::strerror(errno));
//        			close(new_sock_id);
//        			exit(1);
        			continue;
        		}

        		if (msg.tipo == LOGIN) {
        			cli_id = msg.op.login.cli_id;
        			SOCK_CINE_HIJO_LOG("Atiendo cliente %li\n", cli_id);
        		}

        		if (cli_id < 0) {
        			// Nunca me hizo login y no tengo el cli_id
        			SOCK_CINE_HIJO_LOG("Cliente no hizo LOGIN - no tengo su uuid\n");
//        			close(new_sock_id);
//        			exit(1);
        			continue;
        		}
        		else {
        			SOCK_CINE_HIJO_LOG_DEBUG("Recibí pedido %s del cliente\n", strOpType(msg.tipo));

        			msg_queue_send(q_cine_snd, &msg);
        			SOCK_CINE_HIJO_LOG_DEBUG("Envié pedido %s al cine\n", strOpType(msg.tipo));

        			msg_queue_receive(q_cine_rcv, cli_id, &msg);
        			SOCK_CINE_HIJO_LOG_DEBUG("Recibí respuesta %s del cine\n", strOpType(msg.tipo));

        			if (sock_send(new_sock_id, &msg) != sizeof(msg)) {
        				SOCK_CINE_LOG("Error al enviar mensaje al cliente - %s\n", std::strerror(errno));
//            			close(new_sock_id);
//            			exit(1);
        				continue;
        			}
        			SOCK_CINE_HIJO_LOG_DEBUG("Envié respuesta %s al cliente\n", strOpType(msg.tipo));
        		}
        	}
        }

        close(new_sock_id);
    }
}
