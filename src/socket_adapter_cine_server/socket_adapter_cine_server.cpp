#include <csignal>
#include <cstdlib>
#include <cstring>
#include <string>

#include "../common/color_print.h"
#include "../common/ipc/sock.h"

#define ARG_PORT_ADDR	1
#define N_ARGS			2

#define SOCK_CINE_SERVER_LOG(fmt, ...) FPRINTF(stdout, KCYN, fmt, ##__VA_ARGS__)

static int socket_id = -1;

void salir() {
    if (socket_id > 0) {
        close(socket_id);
    }
    SOCK_CINE_SERVER_LOG("SOCKET_ADAPTER_CINE_SERVER finalizado\n");
    exit(0);
}

void handler(int signal) {
	salir();
}

int main(int argc, char *argv[]) {
    if (argc < N_ARGS) {
    	SOCK_CINE_SERVER_LOG("Uso: %s <serv_port>\n", argv[0]);
        exit(1);
    }

    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);
    signal(SIGINT, handler);

    uint16_t serv_port = std::atoi(argv[ARG_PORT_ADDR]);

    socket_id = sock_create();
    if (socket_id == -1 || sock_bind(socket_id, serv_port) != 0 || sock_listen(socket_id, MAX_CLIENTES) != 0) {
    	SOCK_CINE_SERVER_LOG("Error al crear el socket servidor del cine\n");
        salir();
    }

    while (true) {
        int new_sock_id = sock_accept(socket_id);
        if (new_sock_id == -1) {
        	SOCK_CINE_SERVER_LOG("Error al aceptar conexión\n");
            continue;
        }

        pid_t connection_pid = fork();
        if (connection_pid == -1) {
        	SOCK_CINE_SERVER_LOG("Falló el fork del hijo socket_adapter\n");
        }

        if (connection_pid == 0) {
        	execl("./socket_adapter_cine", "socket_adapter_cine", std::to_string(new_sock_id).c_str(), NULL);
        	perror("Error al crear al proceso hijo cine\n");
        	exit(1);
        }
        close(new_sock_id);
    }
}
