#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "../common/canal.h"
#include "../common/operaciones.h"
#include "../common/ipc/msg_queue.h"
#include "../common/colors.h"

void check_timeout(int cli_id, int n_sala, canal *canal_cine_cli, canal *canal_cine_admin) {
    // Con WNOHANG, waitpid devuelve 0 si el hijo no cambió de estado,
    bool timeout = waitpid(-1, NULL, WNOHANG) > 0;
    // si ya terminó, devuelve su pid
    if (timeout) {
        printf("%sTIMEOUT\n", KBLU);
        mensaje_t msg;
        msg.tipo = MSG_TIMEOUT;
        msg.mtype = cli_id;
        msg.op.timeout.cli_id = cli_id;
        msg.op.timeout.n_sala = n_sala;
        // Le aviso al cliente
        canal_enviar(canal_cine_cli, msg);
        // Le aviso al admin
        canal_enviar(canal_cine_admin, msg);
        exit(0);
    }
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

    printf("%sIniciado proceso cine [%i] para cliente [%i]\n", KMAG, getpid(), cli_id);


    mensaje_t msg;
    msg.mtype = cine_id;
    msg.tipo = INFORMAR_SALAS;
    // Le pido al admin la informacion de las salas
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para INFORMAR_SALAS\n", KMAG, getpid());

    canal_recibir(canal_cine_admin, msg, cine_id);
    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);

    // Empieza el timer
    pid_t pid_timer;
    if ((pid_timer = fork()) == 0) {
        execl("./cine_timeout", "cine_timeout", NULL);
        exit(1);
    }
    printf("%sPID_TIMER = %i\n", KBLU, pid_timer);
    // El cliente me pide informacion de una sala especifica
    msg.tipo = ELEGIR_SALA;
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

    // El cliente me pide informacion de una sala especifica
    msg.tipo = ELEGIR_ASIENTOS;
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

    // El cliente me pide informacion de una sala especifica
    msg.tipo = CONFIRMAR_RESERVA;
    canal_recibir(canal_cine_cli, msg, cli_id);
    // Si hubo timeout le aviso al cliente directamente:
    check_timeout(cli_id, -1, canal_cine_cli, canal_cine_admin);
    // Le paso el pedido al admin
    canal_enviar(canal_cine_admin, msg);
    printf("%s[%i] Esperando respuesta del admin para CONFIRMAR_RESERVA\n", KMAG, getpid());
    canal_recibir(canal_cine_admin, msg, cine_id);
    // Le paso al cliente la informacion de la sala
    msg.mtype = cli_id;
    canal_enviar(canal_cine_cli, msg);

    // El cliente me pide informacion de una sala especifica
    msg.tipo = PAGAR;
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
    printf("%s[%i] Terminado\n", KMAG, getpid());
    // Mato al timer para que no siga, ya que no hace falta
    kill(pid_timer, SIGKILL);
}
