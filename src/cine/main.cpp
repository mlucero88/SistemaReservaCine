#include <iostream>
#include <signal.h>
#include "../common/canal.h"
#include "../common/colors.h"
#include "../common/color_print.h"

void info_salas(long cine_id, canal *canal_cine_admin, long cli_id, canal *canal_cine_cli);

// *** ES FEO ESTO PERO NO VEO OTRA FORMA:

canal *canal_cine_admin;
canal *canal_cine_cli;
int cli_id;
int n_sala;

void alarm_handler(int signal) {
    printf("%sTIMEOUT\n", KBLU);
    mensaje_t msg;
    msg.tipo = TIMEOUT;
    msg.mtype = cli_id;
    msg.op.timeout.cli_id = cli_id;
    msg.op.timeout.n_sala = n_sala;
    // Le aviso al cliente

    canal_enviar(canal_cine_cli, msg);

    // Acá se podría poner un msgrcv que NO BLOQUEE, para que saque algun mensaje que el cliente pueda haber mandado
    // Y si no hay nada, no se bloquea

    // Le aviso al admin
    canal_enviar(canal_cine_admin, msg);
    exit(0);
}

void forward_msg(canal *canal_cine_cli, long cli_id, canal *canal_cine_admin, long cine_id, int tipo) {
    mensaje_t msg;
    canal_recibir(canal_cine_cli, msg, cli_id);
    if (msg.tipo == ELEGIR_SALA) {
        // **** HORRIBLE ESTO ****
        printf("NRO DE SALA RECIBIDO: %i\n", msg.op.elegir_sala.nro_sala);
        n_sala = msg.op.elegir_sala.nro_sala;
    }
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para %s\n", KMAG, getpid(), strOpType(tipo));
    canal_recibir(canal_cine_admin, msg, cine_id);
    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);
}

int main(int argc, char *argv[]) {

    struct sigaction sigalarm;
    sigalarm.sa_handler = alarm_handler;
    sigemptyset(&sigalarm.sa_mask);
    sigaction(SIGALRM, &sigalarm, NULL);


    entidad_t cine = {.proceso = entidad_t::CINE, .pid = getpid()};
    entidad_t admin = {.proceso = entidad_t::ADMIN, .pid = -1};

    cli_id = atoi(argv[1]); // id del cliente
    entidad_t cliente = {.proceso = entidad_t::CLIENTE, .pid = cli_id};


    canal_cine_cli = canal_crear(cine, cliente);
    if (canal_cine_cli == NULL) {
        std::cerr << "Error al crear canal de comunicacion entre cine y cliente" << std::endl;
        exit(1);
    }


    canal_cine_admin = canal_crear(cine, admin);
    if (canal_cine_admin == NULL) {
        std::cerr << "Error al crear canal de comunicacion entre cine y cliente" << std::endl;
        exit(1);
    }

    int cine_id = cli_id;

    printf("%sIniciado proceso cine [%i] para cliente [%i]%s\n", KMAG, getpid(), cli_id, KNRM);

    info_salas(cine_id, canal_cine_admin, cli_id, canal_cine_cli);

    alarm(TIMEOUT_VAL);

    forward_msg(canal_cine_cli, cli_id, canal_cine_admin, cine_id, ELEGIR_SALA);
    forward_msg(canal_cine_cli, cli_id, canal_cine_admin, cine_id, ELEGIR_ASIENTOS);
    forward_msg(canal_cine_cli, cli_id, canal_cine_admin, cine_id, CONFIRMAR_RESERVA);
    forward_msg(canal_cine_cli, cli_id, canal_cine_admin, cine_id, PAGAR);

    printf("%s[%i] Terminado%s\n", KMAG, getpid(), KNRM);
}

void info_salas(long cine_id, canal *canal_cine_admin, long cli_id, canal *canal_cine_cli) {
    mensaje_t msg;
    msg.mtype = cine_id;
    msg.tipo = INFORMAR_SALAS;
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para INFORMAR_SALAS%s\n", KMAG, getpid(), KNRM);
    canal_recibir(canal_cine_admin, msg, cine_id);
    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);

}