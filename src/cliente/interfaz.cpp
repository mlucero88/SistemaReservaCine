#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <signal.h>

#include "../common/canal.h"
#include "../common/color_print.h"

#define CLI_LOG cli_log
#define CLI_PRINTF(fmt, ...) FPRINTF(CLI_LOG, BLUE, fmt, ##__VA_ARGS__)

void liberarYSalir() {
    CLI_PRINTF("Preparando para salir...");
    if (pid_asyn > 0) {
        int status;
        kill(pid_asyn, SIGINT);
        wait(&status);
        CLI_PRINTF("Cliente asincronico cerrado");
    }

    if (sharedMem != nullptr) {
        sh_mem_destroy(sharedMem);
        CLI_PRINTF("Memoria compartida liberada");
    }
    if (canal_cli_mom != nullptr) {
        canal_destruir(canal_cli_mom);
        CLI_PRINTF("Canal destruido");
    }

    CLI_PRINTF("*** Aplicacion cerrada ***");
    fclose(cli_log);
    exit(1);
}

int m_init() {
    entidad_t cliente = {.proceso = entidad_t::CLIENTE, .pid = getpid()};
    entidad_t cine = {.proceso = entidad_t::CINE, .pid = -1};
    canal *canal_cli_mom = canal_crear(cliente, mom);
    return (int) (canal_cli_mom);
}

int m_login(int id, int cli_id) {
    mensaje_t msg = {.mtype = LOGIN_MSG_TYPE, .tipo = LOGIN};
    canal *canal_cli_mom = (canal *) id;
    msg.op.login.cli_id = cli_id;
    if (!canal_enviar(canal_cli_mom, msg)) {
        CLI_PRINTF("Error al enviar mensaje de LOGIN: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("LOGIN enviado");
    return 0; // u otra cosa?
}

int m_info_salas(int id, int asientos_por_sala[MAX_SALAS], int *cant_salas) {
    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg;
    // todo cambiar getpid por cli_id
    if (!canal_recibir(canal_cli_mom, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de INFORMAR_SALAS: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != INFORMAR_SALAS) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
            // todo return TIMEOUT??
        } else {
            CLI_PRINTF("Error: se esperaba INFORMAR_SALAS y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("INFORMAR_SALAS recibido");

    memcpy(asientos_por_sala, msg.op.info_salas.asientos_por_sala, MAX_SALAS * sizeof(int));
    *cant_salas = msg.op.info_salas.cant_salas;

    return 0;
}

int m_asientos_sala(int id, int nro_sala, int asientos_sala[MAX_ASIENTOS]) {
    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg = {.mtype = getpid(), .tipo = ELEGIR_SALA}; // todo cambiar getpid por cli_id
    msg.op.elegir_sala.nro_sala = nro_sala;
    if (!canal_enviar(canal_cli_mom, msg)) {
        CLI_PRINTF("Error al enviar mensaje de ELEGIR_SALA: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("ELEGIR_SALA enviado");

    // Esto es lo que estaba en el recibir info sala de la version anterior


    if (!canal_recibir(canal_cli_mom, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de INFORMAR_ASIENTOS: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != INFORMAR_ASIENTOS) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
            // return TIMEOUT
        } else {
            CLI_PRINTF("Error: se esperaba INFORMAR_ASIENTOS y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("INFORMAR_ASIENTOS recibido");

    memcpy(asientos_sala, msg.op.info_asientos.asiento_habilitado, MAX_ASIENTOS * sizeof(int));

    return 0;

}

int m_reservar_asientos(int id, int asientos[MAX_ASIENTOS_RESERVADOS], int cantAsientos, int sala,
                        int asientos_reservados[MAX_ASIENTOS_RESERVADOS], int *cant_reservados) {
    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg = {.mtype = getpid(), .tipo = ELEGIR_ASIENTOS};
    msg.op.elegir_asientos.nro_sala = sala;
    memcpy(msg.op.elegir_asientos.asientos_elegidos, asientos, sizeof(int) * cantAsientos);
    msg.op.elegir_asientos.cant_elegidos = cantAsientos;
    if (!canal_enviar(canal_cli_mom, msg)) {
        CLI_PRINTF("Error al enviar mensaje de ELEGIR_ASIENTOS: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("ELEGIR_ASIENTOS enviado");

    // Esto es lo que estaba en recibir info reserva de la version anterior
    if (!canal_recibir(canal_cli_mom, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de INFORMAR_RESERVA: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != INFORMAR_RESERVA) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
            // return TIMEOUT
            printf("Tardaste mucho :(\n");
        } else {
            CLI_PRINTF("Error: se esperaba INFORMAR_RESERVA y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("INFORMAR_RESERVA recibido");

    memcpy(asientos_reservados, msg.op.info_reserva.asientos_reservados, MAX_ASIENTOS_RESERVADOS * sizeof(int));
    *cant_reservados = msg.op.info_reserva.cant_reservados;

    return 0;

}


int m_confirmar_reserva(int id, bool aceptar, int *precio) {
    // todo cambiar getpid por cli_id
    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg = {.mtype = getpid(), .tipo = CONFIRMAR_RESERVA};
    msg.op.confirmar_reserva.reserva_confirmada = aceptar ? true : false;
    if (!canal_enviar(canal_cli_mom, msg)) {
        CLI_PRINTF("Error al enviar mensaje de CONFIRMAR_RESERVA: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("CONFIRMAR_RESERVA enviado");
    // esto era el recibir info pago de antes

    if (!canal_recibir(canal_cli_mom, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de INFORMAR_PAGO: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != INFORMAR_PAGO) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
            // return TIMEOUT
        } else {
            CLI_PRINTF("Error: se esperaba INFORMAR_PAGO y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("INFORMAR_PAGO recibido");

    *precio = msg.op.info_pago.precio;

    return 0;
}

int m_pagar(int id, int precio) {
    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg = {.mtype = getpid(), .tipo = PAGAR};
    msg.op.pagar.pago = precio;
    if (!canal_enviar(canal_cli_mom, msg)) {
        CLI_PRINTF("Error al enviar mensaje de PAGAR: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("PAGAR enviado");
    // Esto estaba en recibir pago ok en la version anterior

    if (!canal_recibir(canal_cli_mom, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de PAGO_OK: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != PAGO_OK) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
            // return TIMEOUT
        } else {
            CLI_PRINTF("Error: se esperaba PAGO_OK y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("PAGO_OK recibido");
    return 0;
}