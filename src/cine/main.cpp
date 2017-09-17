#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "../common/canal.h"
#include "../common/colors.h"

void elegir_sala(canal *canal_cine_cli, long cli_id, canal *canal_cine_admin, long cine_id);

void elegir_asientos(canal *canal_cine_cli, long cli_id, canal *canal_cine_admin, long cine_id);

void confirmar_reserva(canal *canal_cine_cli, canal *canal_cine_admin, long cine_id, long cli_id);

void pagar(canal *canal_cine_cli, long cli_id, canal *canal_cine_admin, long cine_id);

void info_salas(long cine_id, canal *canal_cine_admin, long cli_id, canal *canal_cine_cli);

pid_t ini_timer();

void check_timeout(int cli_id, int n_sala, canal *canal_cine_cli, canal *canal_cine_admin) {
    // Con WNOHANG, waitpid devuelve 0 si el hijo no cambió de estado,
    bool timeout = waitpid(-1, NULL, WNOHANG) > 0;

    if (!timeout) {
        return;
    }

    printf("%sTIMEOUT\n", KBLU);
    mensaje_t msg;
    msg.tipo = TIMEOUT;
    msg.mtype = cli_id;
    msg.op.timeout.cli_id = cli_id;
    msg.op.timeout.n_sala = n_sala;
    // Le aviso al cliente
    canal_enviar(canal_cine_cli, msg);
    // Le aviso al admin
    canal_enviar(canal_cine_admin, msg);
    exit(0);

}

int main(int argc, char *argv[]) {
    entidad_t cine = {.proceso = entidad_t::CINE, .pid = getpid()};
    entidad_t admin = {.proceso = entidad_t::ADMIN, .pid = -1};
    int cli_id = atoi(argv[1]); // id del cliente
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

    printf("%sIniciado proceso cine [%i] para cliente [%i]%s\n", KMAG, getpid(), cli_id, KNRM);

    info_salas(cine_id, canal_cine_admin, cli_id, canal_cine_cli);

    pid_t pid_timer = ini_timer();

    elegir_sala(canal_cine_cli, cli_id, canal_cine_admin, cine_id);

    elegir_asientos(canal_cine_cli, cli_id, canal_cine_admin, cine_id);

    confirmar_reserva(canal_cine_cli, canal_cine_admin, cine_id, cli_id);

    pagar(canal_cine_cli, cli_id, canal_cine_admin, cine_id);


    printf("%s[%i] Terminado%s\n", KMAG, getpid(), KNRM);
    // Mato al timer para que no siga, ya que no hace falta
    kill(pid_timer, SIGKILL);
}

pid_t ini_timer() {
// Empieza el timer
    pid_t pid_timer;
    if ((pid_timer = fork()) == 0) {
        execl("./cine_timeout", "cine_timeout", NULL);
        exit(1);
    }
    printf("%sPID_TIMER = %i%s\n", KBLU, pid_timer, KNRM);
    return pid_timer;
}

void info_salas(long cine_id, canal *canal_cine_admin, long cli_id, canal *canal_cine_cli) {
    mensaje_t msg;
    msg.mtype = cine_id;
    msg.tipo = INFORMAR_SALAS;

    // Le pido al admin la informacion de las salas
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para INFORMAR_SALAS%s\n", KMAG, getpid(), KNRM);

    canal_recibir(canal_cine_admin, msg, cine_id);

    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);

}

void pagar(canal *canal_cine_cli, long cli_id, canal *canal_cine_admin, long cine_id) {
    // El cliente me pide informacion de una sala especifica
    mensaje_t msg;
    canal_recibir(canal_cine_cli, msg, cli_id);
    // Si hubo timeout le aviso al cliente directamente:
    check_timeout(cli_id, -1, canal_cine_cli, canal_cine_admin);
    // Le paso el pedido al admin
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para PAGAR\n", KMAG, getpid());
    canal_recibir(canal_cine_admin, msg, cine_id);
    // Le paso al cliente la informacion de la sala
    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);
}

void confirmar_reserva(canal *canal_cine_cli, canal *canal_cine_admin, long cine_id, long cli_id) {
    mensaje_t msg;
    // El cliente me pide informacion de una sala especifica
    canal_recibir(canal_cine_cli, msg, cli_id);
    // Si hubo timeout le aviso al cliente directamente:
    check_timeout(cli_id, -1, canal_cine_cli, canal_cine_admin);
    // Le paso el pedido al admin
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para CONFIRMAR_RESERVA\n", KMAG, getpid());
    canal_recibir(canal_cine_admin, msg, cine_id);
    // Le paso al cliente la informacion de la reserva
    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);

}

void elegir_asientos(canal *canal_cine_cli, long cli_id, canal *canal_cine_admin, long cine_id) {
    mensaje_t msg;
    // El cliente me pide informacion de una sala especifica
    canal_recibir(canal_cine_cli, msg, cli_id);
    // Si hubo timeout le aviso al cliente directamente:
    // El cliente
    check_timeout(cli_id, -1, canal_cine_cli, canal_cine_admin);

    // Le paso el pedido al admin
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para ELEGIR_ASIENTOS\n", KMAG, getpid());
    canal_recibir(canal_cine_admin, msg, cine_id);
    // Le paso al cliente la informacion de la sala
    printf("%s[%i] Recibí respuesta del admin para ELEGIR_ASIENTOS\n", KMAG, getpid());
    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);

}

void elegir_sala(canal *canal_cine_cli, long cli_id, canal *canal_cine_admin, long cine_id) {
    mensaje_t msg;
    canal_recibir(canal_cine_cli, msg, cli_id);

    // Si hubo timeout le aviso al cliente directamente:
    // Hasta acá, n_sala = ninguna porque el cliente todavía no eligió
    check_timeout(cli_id, -1, canal_cine_cli, canal_cine_admin);

    // Le paso el pedido al admin
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para INFORMAR_ASIENTOS\n", KMAG, getpid());
    canal_recibir(canal_cine_admin, msg, cine_id);
    // Le paso al cliente la informacion de la sala
    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);

}
