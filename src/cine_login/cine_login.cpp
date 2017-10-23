#include <csignal>
#include <cstdlib>
#include <string>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"

#define ARG_PORT_ADDR	1
#define N_ARGS			2

#define CINE_LOG(fmt, ...) FPRINTF(stdout, KYEL, fmt, ##__VA_ARGS__)

static pid_t admin_pid = -1;
static pid_t adapter_pid = -1;

void salir() {
	if (admin_pid > 0) {
		CINE_LOG("Cerrando ADMIN...\n");
		kill(admin_pid, SIGUSR2);
	}
	if (adapter_pid > 0) {
		CINE_LOG("Cerrando SOCKET_ADAPTER...\n");
		kill(adapter_pid, SIGUSR2);
	}
	CINE_LOG("Proceso finalizado\n");
	exit(0);
}

void handler(int signal) {
	salir();
}

int main(int argc, char* argv[]) {
    if (argc < N_ARGS) {
    	CINE_LOG("Uso: %s <server_port>\n", argv[0]);
        exit(1);
    }

    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);
	signal(SIGINT, handler);

    int q_cli_snd = msg_queue_get(Q_CINE_CLI_B);
    int q_cli_rcv = msg_queue_get(Q_CLI_CINE_B);

	if (q_cli_snd == -1 || q_cli_rcv == -1) {
		CINE_LOG("Error al crear canal de comunicacion entre cine y socket_adapter\n");
		salir();
	}

    admin_pid = fork();
    if (admin_pid == -1) {
    	CINE_LOG("Falló el fork del admin\n");
    	salir();
    }
    else if (admin_pid == 0) {
        execl("./admin", "admin", NULL);
        perror("Error al iniciar el admin\n");
        exit(1);
    }

    adapter_pid = fork();
    if (adapter_pid == -1) {
    	CINE_LOG("Falló el fork del socket_adapter\n");
    	salir();
    }
    else if (adapter_pid == 0) {
        execl("./socket_adapter_cine", "socket_adapter_cine", argv[ARG_PORT_ADDR] ,NULL);
        perror("Error al iniciar el socket_adapter\n");
        exit(1);
    }

    while (true) {
        mensaje_t msg;

        CINE_LOG("Esperando mensaje de login...\n");
        if (msg_queue_receive(q_cli_rcv, LOGIN_MSG_TYPE, &msg)) {
            CINE_LOG("Recibido LOGIN\n");
        	// Creo el proceso que se va a comunicar con este cliente
            pid_t cine_pid = fork();
            if (cine_pid == -1) {
            	CINE_LOG("Falló el fork del hijo cine\n");
            }
        	if (cine_pid == 0) {
        		execl("./cine", "cine", std::to_string(msg.op.login.cli_id).c_str(), NULL);
        		perror("Error al crear al proceso hijo cine\n");
        		exit(1);
        	}
        }
        else {
        	CINE_LOG("Error al recibir mensaje de login\n");
        }
    }
}
