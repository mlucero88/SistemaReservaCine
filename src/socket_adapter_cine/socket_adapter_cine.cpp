#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"
#include "../common/ipc/sock.h"

#define ARG_SOCKET	1
#define N_ARGS		2

#define SOCK_CINE_LOG(fmt, ...) FPRINTF(stdout, KMAG, "[ADAPTER_%li] " fmt, cli_id, ##__VA_ARGS__)
#define SOCK_CINE_SEND_LOG(fmt, ...) FPRINTF(stdout, KMAG, "[ADAPTER_SEND_%li] " fmt, cli_id, ##__VA_ARGS__)
#define SOCK_CINE_RECV_LOG(fmt, ...) FPRINTF(stdout, KMAG, "[ADAPTER_RECV_%li] " fmt, cli_id, ##__VA_ARGS__)

static int socket_id = -1;
static uuid_t cli_id = -1;
static pid_t admin_rcv_pid = -1;
static pid_t cine_rcv_pid = -1;
static bool timeoutReached = false;

void salir() {
	if (admin_rcv_pid > 0) {
		kill(admin_rcv_pid, SIGTERM);
	}
	if (cine_rcv_pid > 0) {
		kill(cine_rcv_pid, SIGTERM);
	}
    if (socket_id > 0) {
        close(socket_id);
    }
    SOCK_CINE_LOG("Proceso finalizado\n");
    exit(0);
}

void handler(int signal) {
	if (signal == SIGUSR2) {
		timeoutReached = true;
		SOCK_CINE_RECV_LOG("Timeout! Descartando paquetes hasta cierre del cliente\n");
	}
	else {
		salir();
	}
}

int main(int argc, char *argv[]) {
    if (argc < N_ARGS) {
    	SOCK_CINE_LOG("Uso: %s <socket_id>\n", argv[0]);
        exit(1);
    }

    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);
    signal(SIGINT, handler);
    signal(SIGUSR2, handler);
    socket_id = std::atoi(argv[ARG_SOCKET]);
    int q_cine_snd = msg_queue_get(Q_CLI_CINE_B);
    int q_cine_rcv = msg_queue_get(Q_CINE_CLI_B);
    int q_admin_cli = msg_queue_get(Q_ADMIN_CLI_B);

    if (q_cine_snd == -1 || q_cine_rcv == -1 || q_admin_cli == -1) {
    	SOCK_CINE_LOG("Error al crear canal de comunicacion entre socket_adapter y cine\n");
        salir();
    }

    mensaje_t msg;

    // Se cierra cuando detecta desconexion del peer
    while (true) {
    	int bytesRec = sock_recv(socket_id, &msg);
    	if (bytesRec == 0) {
    		// Desconexion del peer
    		close(socket_id);
    		salir();
    	} else if (timeoutReached) {
    		// Si hubo timeout, descarto paquetes del cliente y sigo haciendo recv hasta leer el close
    		SOCK_CINE_RECV_LOG("Paquete descartado\n");

    		continue;
    	} else if (bytesRec != sizeof(msg)) {
    		SOCK_CINE_RECV_LOG("Error al recibir mensaje del cliente - %s\n", std::strerror(errno));
    		continue;
    	}

    	if (msg.tipo == LOGIN) {
    		cli_id = msg.op.login.cli_id;

    		admin_rcv_pid = fork();
    		if(admin_rcv_pid == -1) {
    			SOCK_CINE_LOG("Falló el fork del socket_rcv_admin!\n");
    			salir();
    		} else if (admin_rcv_pid == 0) {
    			// Proceso que recibe NOTIFICACIONES
    			signal(SIGINT, SIG_IGN);	// Para no cerrarlo con el CTRL+C. Lo cierra el padre

    			while (true) {
    				SOCK_CINE_SEND_LOG("Esperando notificaciones del admin...\n");
    				msg_queue_receive(q_admin_cli, cli_id, &msg);
    				SOCK_CINE_SEND_LOG("Recibí notificacion %s del admin\n", strOpType(msg.tipo));

    				if (sock_send(socket_id, &msg) != sizeof(msg)) {
    					SOCK_CINE_SEND_LOG("Error al enviar mensaje al cliente - %s\n", std::strerror(errno));
    					continue;
    				}
    				SOCK_CINE_SEND_LOG("Envié notificación %s al cliente\n", strOpType(msg.tipo));
    			}
    		} else {
    			cine_rcv_pid = fork();
    			if(cine_rcv_pid == -1) {
    				SOCK_CINE_LOG("Falló el fork del socker_rcv_cine!\n");
    				salir();
    			} else if (cine_rcv_pid == 0) {
    				// Proceso que recibe respuestas del cine
    				signal(SIGINT, SIG_IGN);	// Para no cerrarlo con el CTRL+C. Lo cierra el padre

    				while (true) {
    					SOCK_CINE_SEND_LOG("Esperando respuesta del cine...\n");
    					msg_queue_receive(q_cine_rcv, cli_id, &msg);
    					SOCK_CINE_SEND_LOG("Recibí respuesta %s del cine\n", strOpType(msg.tipo));
    					if (msg.tipo == TIMEOUT) {
    						kill(getppid(), SIGUSR2);
    					}

    					if (sock_send(socket_id, &msg) != sizeof(msg)) {
    						SOCK_CINE_SEND_LOG("Error al enviar mensaje al cliente - %s\n", std::strerror(errno));
    						continue;
    					}
    					SOCK_CINE_SEND_LOG("Envié respuesta %s al cliente\n", strOpType(msg.tipo));
    				}
    			} else {
    				SOCK_CINE_LOG("Atiendo cliente %li\n", cli_id);
    			}
    		}
    	}

    	if (cli_id < 0) {
    		// Nunca me hizo login y no tengo el cli_id
    		SOCK_CINE_RECV_LOG("Cliente no hizo LOGIN - no tengo su uuid\n");
    		continue;
    	} else {
    		SOCK_CINE_RECV_LOG("Recibí pedido %s del cliente\n", strOpType(msg.tipo));
    		msg_queue_send(q_cine_snd, &msg);
    		SOCK_CINE_RECV_LOG("Envié pedido %s al cine\n", strOpType(msg.tipo));
    	}
    }
}
