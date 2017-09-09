#include <random>
#include <cerrno>
#include <cstring>
#include <iostream>

#include "../common/constantes.h"
#include "../common/entidad.h"
#include "../common/canal.h"
#include "../common/operaciones.h"
#include "../common/ipc/msg_queue.h"

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
        if (asiento_habilitado[id_asiento]) {
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

void login(canal *canal_cli_cine) {
    mensaje_t msg;
    msg.tipo = LOGIN;
    msg.operacion.login.cli_id = getpid();
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
        std::cerr << "Error al recibir mensaje de INFORMAR_SALAS: " << strerror(errno) << std::endl;
        exit(1);
    }
    printf("[%i] Recibí INFORMAR_SALAS\n", getpid());
    return msg;
}

void elegir_sala(canal *canal_cli_cine, mensaje_t msg_prev) {
    int cli_id = getpid();
    mensaje_t msg;
    msg.tipo = ELEGIR_SALA;
    msg.mtype = cli_id;
    msg.operacion.elegir_sala.nro_sala = elegir_sala_rand(msg_prev.operacion.informar_salas.asientos_por_sala,
                                                          msg_prev.operacion.informar_salas.cantidad_salas);
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
        std::cerr << "Error al recibir mensaje de INFORMAR_ASIENTOS" << std::endl;
        exit(1);
    }
    printf("[%i] Recibí INFORMAR_ASIENTOS\n", getpid());
    return msg;
}

void elegir_asientos(canal *canal_cli_cine, mensaje_t msg_prev) {
    int cli_id = getpid();
    int asientos[MAX_ASIENTOS_RESERVADOS];
    int cantidad = elegir_asientos_rand(msg_prev.operacion.informar_asientos.asiento_habilitado,
                                        msg_prev.operacion.informar_asientos.cantidad_asientos,
                                        asientos);
    mensaje_t msg;
    msg.tipo = ELEGIR_ASIENTOS;
    msg.mtype = cli_id;
    memcpy(msg.operacion.elegir_asientos.asientos_elegidos, asientos, sizeof(int) * cantidad);
    msg.operacion.elegir_asientos.cantidad_elegidos = cantidad;
    printf("[%i] ELEGIR_ASIENTOS\n", getpid());
    if (!canal_enviar(canal_cli_cine, msg)) {
        std::cerr << "Error al enviar mensaje de LOGIN: " << strerror(errno) << std::endl;
        exit(1);
    }
}

void recibir_info_reserva(canal *canal_cli_cine) {
    mensaje_t msg;
    int cli_id = getpid();
    if (!canal_recibir(canal_cli_cine, msg, cli_id) || msg.tipo != INFORMAR_RESERVA) {
        std::cerr << "Error al recibir mensaje de INFORMAR_RESERVA" << std::endl;
        exit(1);
    }
    printf("[%i] Recibí INFORMAR_RESERVA\n", getpid());
}

void confirmar_reserva(canal *canal_cli_cine) {
    mensaje_t msg;
    int cli_id = getpid();
    msg.tipo = CONFIRMAR_RESERVA;
    msg.mtype = cli_id;
    msg.operacion.confirmar_reserva.reserva_confirmada = 1; // ((rand() % 10) > 0);    /* 9/10 de ser confirmada */
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
    msg.operacion.pagar.pago = msg.operacion.informar_pago.precio;
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

    // Elegir una sala de las que recibir antes
    elegir_sala(canal_cli_cine, msg);

    msg = recibir_info_sala(canal_cli_cine);

    // Elegir asientos de la sala elegida
    elegir_asientos(canal_cli_cine, msg);

    recibir_info_reserva(canal_cli_cine);

    confirmar_reserva(canal_cli_cine);

    recibir_info_pago(canal_cli_cine);

    pagar(canal_cli_cine);

    recibir_pago_ok(canal_cli_cine);

    canal_destruir(canal_cli_cine);

    return 0;
}
