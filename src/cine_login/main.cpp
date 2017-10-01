#include <csignal>
#include <cstring>
#include <iostream>

#include "../common/canal.h"


int main() {

    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);

    entidad_t cine = {.proceso = entidad_t::CINE, .pid = getpid()};
    entidad_t cliente = {.proceso = entidad_t::CLIENTE, .pid = -1};

    canal *canal_cine_cli = canal_crear(cine, cliente);
    if (canal_cine_cli == NULL) {
        std::cerr << "Error al crear canal de comunicacion entre cine y cliente" << std::endl;
        exit(1);
    }

    if (fork() == 0) {
        execl("./admin", "admin", NULL);
        perror("Error al iniciar el admin");
        exit(1);
    }

    while (true) {
        mensaje_t msg;
        msg.tipo = LOGIN;

        printf("Esperando mensaje de login\n");
        int r = canal_recibir(canal_cine_cli, msg, LOGIN_MSG_TYPE);
        if (r != -1) {
            // Creo el proceso que se va a comunicar con este cliente
            if (fork() == 0) {
                // id del cliente que se logueó, puede o no ser el pid
                int cli_id = msg.op.login.cli_id;
                char cli_id_str[12];
                sprintf(cli_id_str, "%d", cli_id);
                execl("./cine", "cine", cli_id_str, NULL);
                perror("Error al crear al proceso hijo cine");
                exit(1);
            }
        } else {
            perror("Error al recibir mensaje de login");
            exit(1);
        }

    }
}
