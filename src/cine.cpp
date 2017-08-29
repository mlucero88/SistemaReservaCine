#include "commons.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>

// handler para sigchild no hacer nada

int handler(int sig){}

int main() {
    int msg_cine_cli = msg_get(Q_CINE_CLI);

    if (msg_cine_cli == -1) {
        perror("Error cine_cli:");
        exit(1);
    }

    int msg_cli_cine = msg_get(Q_CLI_CINE);

    if (msg_cli_cine == -1) {
        perror("Error cli_cine:");
        exit(1);
    }

    // registrar handler

    while (true) {
        struct msg m;
        int r = msgrcv(msg_cli_cine, &m, sizeof(m) - sizeof(long), LOGIN, 0);
        if (r == -1) {
            // ERROR
            perror("Error recibir cli_cine:");
            exit(1);
        }
        if (m.type == LOGIN) {
            // Crear el otro cine
            if (fork() == 0) {
                exec("",m.op.login.id);
                perror("Error exec");
                exit(1);
            }
        }

    }
}