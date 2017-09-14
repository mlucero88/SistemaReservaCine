#include <iostream>
#include <signal.h>
#include "../common/canal.h"
#include "../common/operaciones.h"
#include "../common/ipc/msg_queue.h"

#define TIMEOUT 60

void alarm_handler(int) {
    // avisarle al cliente para que se cierre
    mensaje_t msg;
    msg.tipo = MSG_TIMEOUT;
    msg.mtype = cli_id;
    msg.op; // No hace falta nada más
    canal_enviar(canal_cine_cli, msg);
    // avisarle al admin para que cancele las reservas si se hizo alguna
    msg.mtype = cli_id;
    canal_enviar(canal_cine_admin, msg);
}


int main(int argc, char *argv[]) {
    struct sigaction sigchld, sigalarm;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);

    sigalarm.sa_handler = alarm_handler;
    sigemptyset(&sigalarm.sa_mask);
    sigaction(SIGALRM, &sigalarm, NULL);

    alarm(TIMEOUT);


    entidad_t cine = {.proceso = entidad_t::CINE, .pid = getpid()};
    entidad_t admin = {.proceso = entidad_t::ADMIN, .pid = -1};
    int cli_id = atoi(argv[0]); // id del cliente
    entidad_t cliente = {.proceso = entidad_t::CLIENTE, .pid = cli_id};

    canal *canal_cine_cli = canal_crear(cine, cliente);
    if (canal_cine_cli == NULL) {
        std::cerr << "Error al crear canal de comunicacion entre cine y cliente" << std::endl;
        exit(1);
    }

    canal *canal_cine_admin = canal_crear(cine, admin);
    if (canal_cine_admin == NULL) {
        std::cerr << "Error al crear canal de comunicacion entre cine y cliente" << std::endl;
        exit(1);
    }

    int cine_id = cli_id;

    printf("Iniciado proceso cine [%i] para cliente [%i]\n", getpid(), cli_id);

    while (true) {
        mensaje_t msg;
        msg.mtype = cine_id;
        msg.tipo = INFORMAR_SALAS;
        // Le pido al admin la informacion de las salas
        canal_enviar(canal_cine_admin, msg);
        printf("[%i] Esperando respuesta del admin para INFORMAR_SALAS\n", getpid());

        canal_recibir(canal_cine_admin, msg, cine_id);
        // Le reenvio la informacion al cliente
        printf("m.tipo = %i\n", msg.tipo);
        msg.mtype = cli_id;
        canal_enviar(canal_cine_cli, msg);

        // El cliente me pide informacion de una sala especifica
        msg.tipo = ELEGIR_SALA;
        canal_recibir(canal_cine_cli, msg, cli_id);
        // Le paso el pedido al admin
        canal_enviar(canal_cine_admin, msg);
        printf("[%i] Esperando respuesta del admin para INFORMAR_ASIENTOS\n", getpid());
        canal_recibir(canal_cine_admin, msg, cine_id);
        // Le paso al cliente la informacion de la sala
        msg.mtype = cli_id;
        canal_enviar(canal_cine_cli, msg);

        // El cliente me pide informacion de una sala especifica
        msg.tipo = ELEGIR_ASIENTOS;
        canal_recibir(canal_cine_cli, msg, cli_id);
        // Le paso el pedido al admin
        canal_enviar(canal_cine_admin, msg);
        printf("[%i] Esperando respuesta del admin para ELEGIR_ASIENTOS\n", getpid());
        canal_recibir(canal_cine_admin, msg, cine_id);
        // Le paso al cliente la informacion de la sala
        printf("[%i] Recibí respuesta del admin para ELEGIR_ASIENTOS\n", getpid());
        msg.mtype = cli_id;
        canal_enviar(canal_cine_cli, msg);

        // El cliente me pide informacion de una sala especifica
        msg.tipo = CONFIRMAR_RESERVA;
        canal_recibir(canal_cine_cli, msg, cli_id);
        // Le paso el pedido al admin
        canal_enviar(canal_cine_admin, msg);
        printf("[%i] Esperando respuesta del admin para CONFIRMAR_RESERVA\n", getpid());
        canal_recibir(canal_cine_admin, msg, cine_id);
        // Le paso al cliente la informacion de la sala
        msg.mtype = cli_id;
        canal_enviar(canal_cine_cli, msg);

        // El cliente me pide informacion de una sala especifica
        msg.tipo = PAGAR;
        canal_recibir(canal_cine_cli, msg, cli_id);
        // Le paso el pedido al admin
        canal_enviar(canal_cine_admin, msg);
        printf("[%i] Esperando respuesta del admin para PAGAR\n", getpid());
        canal_recibir(canal_cine_admin, msg, cine_id);
        // Le paso al cliente la informacion de la sala
        msg.mtype = cli_id;
        canal_enviar(canal_cine_cli, msg);
        printf("[%i] Terminado\n", getpid());
        break;
    }

}
