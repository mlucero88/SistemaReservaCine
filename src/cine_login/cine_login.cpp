#include <csignal>
#include <cstdlib>

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"

#define CINE_LOG(fmt, ...) FPRINTF(stdout, KYEL, fmt, ##__VA_ARGS__)

int main(int argc, char* argv[]) {
    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);

	int q_cli_snd = msg_queue_get(Q_CINE_CLI);
	int q_cli_rcv = msg_queue_get(Q_CLI_CINE);

	if (q_cli_snd == -1 || q_cli_rcv == -1) {
		CINE_LOG("Error al crear canal de comunicacion entre cine y cliente\n");
		exit(1);
	}

    if (fork() == 0) {
        execl("./admin", "admin", NULL);
        perror("Error al iniciar el admin\n");
        exit(1);
    }

    while (true) {
        mensaje_t msg;
        msg.tipo = LOGIN;

        CINE_LOG("Esperando mensaje de login...\n");
        if (msg_queue_receive(q_cli_rcv, LOGIN_MSG_TYPE, &msg)) {
            CINE_LOG("Recibido LOGIN\n");
        	// Creo el proceso que se va a comunicar con este cliente
        	if (fork() == 0) {
        		// id del cliente que se logueó, puede o no ser el pid
        		int cli_id = msg.op.login.cli_id;
        		char cli_id_str[12];
        		sprintf(cli_id_str, "%d", cli_id);
        		execl("./cine", "cine", cli_id_str, NULL);
        		perror("Error al crear al proceso hijo cine\n");
        		exit(1);
        	}
        }
        else {
        	CINE_LOG("Error al recibir mensaje de login\n");
        	exit(1);
        }
    }
}
