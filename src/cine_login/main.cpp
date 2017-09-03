#include <csignal>
#include <cstring>
#include <iostream>

#include "common/canal_comunicacion.h"

void alarm_handler(int) {
}

int main() {
	struct sigaction sigchld, sigalarm;
	sigchld.sa_handler = SIG_DFL;
	sigchld.sa_flags = SA_NOCLDWAIT;
	sigemptyset(&sigchld.sa_mask);
	sigaction(SIGCHLD, &sigchld, NULL);

	sigalarm.sa_handler = alarm_handler;
	sigemptyset(&sigalarm.sa_mask);
	sigaction(SIGALRM, &sigalarm, NULL);

	entidad_t cine = { .proceso = entidad_t::CINE, .pid = getpid() };
	entidad_t cliente = { .proceso = entidad_t::CLIENTE, .pid = -1 };

	canal_comunicacion* canal_cine_cli = canal_comunicacion_crear(cine, cliente);
	if (canal_cine_cli == NULL) {
		std::cerr << "Error al crear canal de comunicacion entre cine y cliente" << std::endl;
		exit(1);
	}

//	while (true) {
//		struct msg m;
//		int r = msgrcv(msg_cli_cine, &m, sizeof(m) - sizeof(long), LOGIN, 0);
//		if (r == -1) {
//			// ERROR
//			perror("Error recibir cli_cine:");
//			exit(1);
//		}
//		if (m.type == LOGIN) {
//			// Crear el otro cine
//			if (fork() == 0) {
//				exec("", m.op.login.id);
//				perror("Error exec");
//				exit(1);
//			}
//		}
//	}

	canal_comunicacion_destruir(canal_cine_cli);
	return 0;

}
