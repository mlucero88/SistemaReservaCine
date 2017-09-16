#include <random>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "../common/constantes.h"
#include "../common/entidad.h"
#include "../common/canal.h"
#include "../common/operaciones.h"
#include "../common/ipc/msg_queue.h"
#include "../common/colors.h"

static int elegir_sala_rand(const int *asientos_por_sala, int cantidad_salas) {
    int sala;
    do {
        sala = rand() % cantidad_salas;
    } while (asientos_por_sala[sala] < 1);
    return sala;
}

static int elegir_asientos_rand(const int *asiento_habilitado, int cantidad_asientos, int *asientos_elegidos) {
    int rounds = 1 + (rand() % MAX_ASIENTOS_RESERVADOS);
    int id_asiento;
    int i = 0;
    int intentos = 20; /* Corte por si no hay suficientes asientos */

    while (rounds > 0 && intentos > 0) {
        id_asiento = rand() % cantidad_asientos;
        if (asiento_habilitado[id_asiento] == DISPONIBLE) {
            /* Reviso que no lo haya elegido en un round anterior */
            bool ya_elegido = false;
            for (int j = 0; j < i; ++j) {
                if (id_asiento == asientos_elegidos[j]) {
                    ya_elegido = true;
                    break;
                }
            }
            if (!ya_elegido) {
                asientos_elegidos[i++] = id_asiento;
                --rounds;
            }
        }
        --intentos;
    }

    return i;
}

pid_t pid_asyn;

void handle_timeout() {
    printf("%sRECIBI TIMEOUT\n", KBLU);
    kill(pid_asyn, SIGKILL); // TODO LIBERAR MEM COMPARTIDA
}

void login(canal *canal_cli_cine) {
    mensaje_t msg;
    msg.tipo = LOGIN;
    msg.op.login.cli_id = getpid();
    msg.mtype = LOGIN_MSG_TYPE;
    printf("[%i] LOGIN\n", getpid());
    if (!canal_enviar(canal_cli_cine, msg)) {
        std::cerr << "Error al enviar mensaje de LOGIN: " << strerror(errno) << std::endl;
        exit(1);
    }
}

mensaje_t recibir_info_salas(canal *canal_cli_cine) {
    int cli_id = getpid();
    mensaje_t msg;

    if (!canal_recibir(canal_cli_cine, msg, cli_id) || msg.tipo != INFORMAR_SALAS) {
        if (msg.tipo == TIMEOUT) {
            handle_timeout();
        }
        std::cerr << "Error al recibir mensaje de INFORMAR_SALAS: " << strerror(errno) << std::endl;
        exit(1);
    }
    printf("[%i] Recibí INFORMAR_SALAS\n", getpid());
    printf("Cantidad de salas: %i\n", msg.op.info_salas.cant_salas);
    for (int i = 0; i < msg.op.info_salas.cant_salas; i++) {
        printf("SALA %i -> %i\n", i + 1, msg.op.info_salas.asientos_por_sala[i]);
    }

    return msg;
}

void elegir_sala(canal *canal_cli_cine, mensaje_t msg_prev) {
    int cli_id = getpid();
    mensaje_t msg;
    msg.tipo = ELEGIR_SALA;
    msg.mtype = cli_id;
    msg.op.elegir_sala.nro_sala = elegir_sala_rand(msg_prev.op.info_salas.asientos_por_sala,
                                                   msg_prev.op.info_salas.cant_salas);
    printf("[%i] ELEGIR_SALA\n", getpid());
    if (!canal_enviar(canal_cli_cine, msg)) {
        std::cerr << "Error al enviar mensaje de ELEGIR_SALA: " << strerror(errno) << std::endl;
        exit(1);
    }
}

mensaje_t recibir_info_sala(canal *canal_cli_cine) {
    mensaje_t msg;
    int cli_id = getpid();
    if (!canal_recibir(canal_cli_cine, msg, cli_id) || msg.tipo != INFORMAR_ASIENTOS) {
        if (msg.tipo == MSG_TIMEOUT) {
            handle_timeout();
        }
        std::cerr << "Error al recibir mensaje de INFORMAR_ASIENTOS" << std::endl;
        exit(1);
    }
    printf("[%i] Recibí INFORMAR_ASIENTOS\n", getpid());
    printf("Hay %i asientos en total en la sala\n", msg.op.info_asientos.cant_asientos);
    for (int j = 0; j < MAX_ASIENTOS; j++) {
        printf("%c", msg.op.info_asientos.asiento_habilitado[j] == DISPONIBLE ? 'O' : 'X');
    }
    printf("\n");

    return msg;
}

void elegir_asientos(canal *canal_cli_cine, mensaje_t msg_prev) {
    int cli_id = getpid();
    int asientos[MAX_ASIENTOS_RESERVADOS];
    // TODO LEER DE LA MEMORIA COMPARTIDA LOS ASIENTOS QUE HAY
    int cantidad = elegir_asientos_rand(msg_prev.op.info_asientos.asiento_habilitado,
                                        msg_prev.op.info_asientos.cant_asientos,
                                        asientos);
    mensaje_t msg;
    msg.tipo = ELEGIR_ASIENTOS;
    msg.mtype = cli_id;
    msg.op.elegir_asientos.nro_sala = msg_prev.op.info_asientos.nro_sala;
    memcpy(msg.op.elegir_asientos.asientos_elegidos, asientos, sizeof(int) * cantidad);
    msg.op.elegir_asientos.cant_elegidos = cantidad;
    printf("[%i] ELEGIR_ASIENTOS\n", getpid());
    if (!canal_enviar(canal_cli_cine, msg)) {
        std::cerr << "Error al enviar mensaje de LOGIN: " << strerror(errno) << std::endl;
        exit(1);
    }
    printf("Reservar %i asientos\n", cantidad);
    printf("Asientos:\n");
    for (int i = 0; i < cantidad; i++) {
        printf("%i\n", msg.op.elegir_asientos.asientos_elegidos[i]);
    }
}

void recibir_info_reserva(canal *canal_cli_cine) {
    mensaje_t msg;
    int cli_id = getpid();
    if (!canal_recibir(canal_cli_cine, msg, cli_id) || msg.tipo != INFORMAR_RESERVA) {
        if (msg.tipo == MSG_TIMEOUT) {
            handle_timeout();
        }
        std::cerr << "Error al recibir mensaje de INFORMAR_RESERVA" << std::endl;
        exit(1);
    }
    printf("[%i] Recibí INFORMAR_RESERVA\n", getpid());
    int nro_reservados = msg.op.info_reserva.cant_reservados;
    printf("Se reservaron %i asientos\n", nro_reservados);
    printf("Se reservaron los siguientes asientos:\n");
    for (int i = 0; i < nro_reservados; i++) {
        printf("%i\n", msg.op.info_reserva.asientos_reservados[i]);
    }
}

void confirmar_reserva(canal *canal_cli_cine) {
    mensaje_t msg;
    int cli_id = getpid();
    msg.tipo = CONFIRMAR_RESERVA;
    msg.mtype = cli_id;
    msg.op.confirmar_reserva.reserva_confirmada = 1; // ((rand() % 10) > 0);    /* 9/10 de ser confirmada */
    printf("[%i] CONFIRMAR_RESERVA\n", getpid());
    if (!canal_enviar(canal_cli_cine, msg)) {
        std::cerr << "Error al enviar mensaje de CONFIRMAR_RESERVA: " << strerror(errno) << std::endl;
        exit(1);
    }

}

void recibir_info_pago(canal *canal_cli_cine) {
    mensaje_t msg;
    int cli_id = getpid();
    if (!canal_recibir(canal_cli_cine, msg, cli_id) || msg.tipo != INFORMAR_PAGO) {
        if (msg.tipo == TIMEOUT) {
            handle_timeout();
        }
        std::cerr << "Error al recibir mensaje de INFORMAR_PAGO" << std::endl;
        exit(1);
    }
    printf("[%i] Recibí INFORMAR_PAGO\n", getpid());

}

void pagar(canal *canal_cli_cine) {
    mensaje_t msg;
    int cli_id = getpid();
    /* Envio pago */
    msg.tipo = PAGAR;
    msg.mtype = cli_id;
    msg.op.pagar.pago = msg.op.info_pago.precio;
    printf("[%i] PAGAR\n", getpid());
    if (!canal_enviar(canal_cli_cine, msg)) {
        std::cerr << "Error al enviar mensaje de PAGAR: " << strerror(errno) << std::endl;
        exit(1);
    }
}

void recibir_pago_ok(canal *canal_cli_cine) {
    mensaje_t msg;
    int cli_id = getpid();
    if (!canal_recibir(canal_cli_cine, msg, cli_id) || msg.tipo != PAGO_OK) {
        if (msg.tipo == MSG_TIMEOUT) {
            handle_timeout();
        }
        std::cerr << "Error al recibir mensaje de INFORMAR_PAGO" << std::endl;
        exit(1);
    }
    printf("[%i] Recibí PAGO_OK\n", getpid());

}

int main() {
    int cli_id = getpid();
    entidad_t cliente = {.proceso = entidad_t::CLIENTE, .pid = cli_id};
    entidad_t cine = {.proceso = entidad_t::CINE, .pid = -1};

    canal *canal_cli_cine = canal_crear(cliente, cine);
    if (canal_cli_cine == NULL) {
        std::cerr << "Error al crear canal de comunicacion entre cliente y cine" << std::endl;
        exit(1);
    }

    srand(time(NULL));

    login(canal_cli_cine);

    mensaje_t msg;

    msg = recibir_info_salas(canal_cli_cine);

    key_t key = ftok("/media", cli_id);
    int shmid = shmget(key, sizeof(int) * MAX_ASIENTOS, 0666 | IPC_CREAT | IPC_EXCL);
    // TODO liberar esta memoria al terminar!!!
    if ((pid_asyn = fork()) == 0) {
        // Lanzo cliente asyn
        char cli_id_str[12];
        sprintf(cli_id_str, "%d", cli_id);
        execl("./client_asyn", "client_asyn", cli_id_str, NULL);
        exit(1);
    }

    printf("%sCLIENTE ASYN PARA CLIENTE %i = %i%s\n", KRED, getpid(), pid_asyn, KNRM);

    // Elegir una sala de las que recibi antes

    getchar(); // comentar para que sea automatico

    //elegir_sala(canal_cli_cine, msg);
    msg.tipo = ELEGIR_SALA;
    msg.mtype = cli_id;
    msg.op.elegir_sala.nro_sala = 1;
    printf("[%i] ELEGIR_SALA\n", getpid());
    if (!canal_enviar(canal_cli_cine, msg)) {
        std::cerr << "Error al enviar mensaje de ELEGIR_SALA: " << strerror(errno) << std::endl;
        exit(1);
    }


    msg = recibir_info_sala(canal_cli_cine);

    // Elegir asientos de la sala elegida

    getchar(); // comentar para que sea automatico
    elegir_asientos(canal_cli_cine, msg);

    recibir_info_reserva(canal_cli_cine);


    getchar(); // comentar para que sea automatico
    confirmar_reserva(canal_cli_cine);

    recibir_info_pago(canal_cli_cine);

    getchar(); // comentar para que sea automatico
    pagar(canal_cli_cine);

    recibir_pago_ok(canal_cli_cine);

    canal_destruir(canal_cli_cine);

    return 0;
}
