#include <cerrno>
#include <cstring>
#include <cstdlib>

#include "interfaz.h"

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"

#define INT_LOG stdin
#define INT_PRINTF(fmt, ...) FPRINTF(INT_LOG, BLUE, fmt, ##__VA_ARGS__)

int m_errno;

//int enviar_msj(mensaje_t &msg, canal *canal_cli_mom) {
//    if (!canal_enviar(canal_cli_mom, msg)) {
//        INT_PRINTF("Error al enviar mensaje de %s: %s", strOpType(msg.tipo), strerror(errno));
//        return RET_ERROR;
//    }
//    INT_PRINTF("%s enviado", strOpType(msg.tipo));
//    return RET_OK;
//}
//
//int recibir_msj(mensaje_t &msg, canal *canal_cli_mom, int tipo) {
//    // todo ver si está bien getpid() o va otra cosa en su lugar
//    if (!canal_recibir(canal_cli_mom, msg, getpid())) {
//        INT_PRINTF("Error al recibir mensaje de %s: %s", strOpType(msg.tipo), strerror(errno));
//        return RET_ERROR;
//    }
//    if (msg.tipo != tipo) {
//        if (msg.tipo == TIMEOUT) {
//            INT_PRINTF("TIMEOUT recibido");
//            return RET_TIMEOUT;
//        } else {
//            char err_str[128];
//            strcpy(err_str, strerror(errno));
//            char txt[256];
//            strcpy(txt, "Error: se esperaba $s y se recibió %s ");
//            strcat(txt, strOpType(msg.tipo));
//            strcat(txt, " ");
//            strcat(txt, strOpType(msg.tipo));
//            INT_PRINTF("%s", txt);
//            return RET_ERROR;
//            // Esto no funciona: INT_PRINTF("Error al recibir mensaje de %s: %s", strOpType(msg.tipo), strerror(errno));
//            // En vez de liberarYSalir, que directamente cerraba el cliente, como ahora esto está separado del cliente
//            // no puede cerrarlo a la fuerza, mejor que devuelva ERROR, y que después se destruya esto desde afuera con
//            // m_dest()
//        }
//        return RET_ERROR;
//    }
//
//    INT_PRINTF("%s recibido", strOpType(tipo));
//    return RET_OK;
//}


/* Inicia el MOM cliente. */
m_id m_init() {
//    if (fork() == 0) {
//        execl("./mom_cliente", "mom_cliente", NULL);
//        perror("Error al iniciar el MOM: ");
//        exit(1);
//    }
//    canal *canal_cli_mom = canal_crear(proceso_t::CLIENTE, proceso_t::MOM);
//    return (void *) (canal_cli_mom);
	m_errno = RET_OK;
	return NULL;
}

/* Destruye el MOM cliente. */
void m_dest(m_id id) {
//    canal *canal_cli_mom = (canal *) id;
//    canal_destruir(canal_cli_mom);
    // Acá va el wait() para el MOM
	m_errno = RET_OK;
}


/* Loguea al cliente. */
void m_login(m_id id, int cli_id) {
//    mensaje_t msg = {.mtype = LOGIN_MSG_TYPE, .tipo = LOGIN};
//    canal *canal_cli_mom = (canal *) id;
//    msg.op.login.cli_id = cli_id;
//    return enviar_msj(msg, canal_cli_mom);
	m_errno = RET_OK;
}

/* Carga en asientos_por_sala[i] la cantidad de asientos que hay en la sala [i], y en cant_salas la cantidad de salas
 * que hay en total. */
int m_info_salas(m_id id, int asientos_por_sala[MAX_SALAS]) {
//    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg;
//    int r;
//    r = recibir_msj(msg, canal_cli_mom, INFORMAR_SALAS);
//    if (r != RET_OK) {
//        return r;
//    }
//
//    memcpy(asientos_por_sala, msg.op.info_salas.asientos_por_sala, MAX_SALAS * sizeof(int));
	m_errno = RET_OK;
    return msg.op.info_salas.cant_salas;
}

/*Carga en asientos_sala los asientos que están DISPONIBLE o NO_DISPONIBLE para la sala nro_sala. */

int m_asientos_sala(m_id id, int nro_sala, int asientos_habilitados[MAX_ASIENTOS]) {
//    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg = {.mtype = getpid(), .tipo = ELEGIR_SALA}; // todo cambiar getpid por cli_id
//    msg.op.elegir_sala.nro_sala = nro_sala;
//
//    if (enviar_msj(msg, canal_cli_mom) == RET_ERROR) {
//        return RET_ERROR;
//    }
//
//    int r;
//    r = recibir_msj(msg, canal_cli_mom, INFORMAR_ASIENTOS);
//    if (r != RET_OK) {
//        return r;
//    }
//
//    memcpy(asientos_habilitados, msg.op.info_asientos.asiento_habilitado, MAX_ASIENTOS * sizeof(int));
    m_errno = RET_OK;
    return msg.op.info_asientos.cant_asientos;
}

/* Intenta reservar los asientos en la sala indicada. Recibe la cantidad de asientos a reservar y el número de la sala.
 * Además, pone en asientos_reservados[i] = 1 si el asiento en asientos[i] pudo ser reservado o 0 si no. En cant_reservados,
 * se indica la cantidad de asientos que sí pudieron ser reservados. */
int m_reservar_asientos(m_id id, int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos, int nro_sala,
                        int asientos_reservados[MAX_ASIENTOS_RESERVADOS]) {
//    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg = {.mtype = getpid(), .tipo = ELEGIR_ASIENTOS};
//    msg.op.elegir_asientos.nro_sala = nro_sala;
//    memcpy(msg.op.elegir_asientos.asientos_elegidos, asientos_elegidos, sizeof(int) * cant_elegidos);
//    msg.op.elegir_asientos.cant_elegidos = cant_elegidos;
//
//    if (enviar_msj(msg, canal_cli_mom) == RET_ERROR) {
//        return RET_ERROR;
//    }
//
//    int r;
//    r = recibir_msj(msg, canal_cli_mom, INFORMAR_RESERVA);
//    if (r != RET_OK) {
//        return r;
//    }
//
//    memcpy(asientos_reservados, msg.op.info_reserva.asientos_reservados, MAX_ASIENTOS_RESERVADOS * sizeof(int));
    m_errno = RET_OK;
    return msg.op.info_reserva.cant_reservados;
}


/* Envia confirmación, o no, según indique aceptar. Si se acepta la reserva, se carga el precio a pagar en precio. */
int m_confirmar_reserva(m_id id, bool aceptar) {
    // todo cambiar getpid por cli_id
//    canal *canal_cli_mom = (canal *) id;
    mensaje_t msg = {.mtype = getpid(), .tipo = CONFIRMAR_RESERVA};
//
//    msg.op.confirmar_reserva.reserva_confirmada = aceptar;
//    if (enviar_msj(msg, canal_cli_mom) == RET_ERROR) {
//        return RET_ERROR;
//    }
//
//    int r;
//    r = recibir_msj(msg, canal_cli_mom, INFORMAR_PAGO);
//    if (r != RET_OK) {
//        return r;
//    }

    m_errno = RET_OK;
    return msg.op.info_pago.precio;
}

/* Realiza el pago para la reserva. Se pasa el precio a pagar. */
void m_pagar(m_id id, int precio) {
//    canal *canal_cli_mom = (canal *) id;
//    mensaje_t msg = {.mtype = getpid(), .tipo = PAGAR};
//    msg.op.pagar.pago = precio;
//    if (enviar_msj(msg, canal_cli_mom) == RET_ERROR) {
//        return RET_ERROR;
//    }
//
//    return recibir_msj(msg, canal_cli_mom, PAGO_OK);
	m_errno = RET_OK;
}

const char* m_str_error(int err) {
    switch (err) {
        case ERR_MOMID: {
            return "Invalid mom_id";
        }
        case ERR_MSGSEND: {
            return "Cannot send message";
        }
        case ERR_MSGRECV: {
            return "Cannot receive message";
        }
        case ERR_OPINVALID: {
            return "Cannot perform this operation";
        }
        case ERR_SALAINVALID: {
            return "ELEGIR_ASIENTOS";
        }
        case ERR_TIMEOUT: {
            return "INFORMAR_RESERVA";
        }
        case RET_OK: {
            return "INFORMAR_RESERVA";
        }
        default: {
            break;
        }
    }
    return "";
}
