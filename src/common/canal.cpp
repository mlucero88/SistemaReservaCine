#include <cerrno>
#include <cstring>
#include <iostream>

#include "ipc/msg_queue.h"
#include "canal.h"

struct canal {
	entidad_t local;
	entidad_t remoto;
	int id_queue_envio;
	int id_queue_recepcion;
};

/* Otra opcion es hacer una funcion para crear un canal especifico
 * Ej: canal_comunicacion_crear_cine_cliente()
 * Pero como está ahora es mas versatil, aunque sean feos los ifs */
canal *canal_crear(entidad_t local, entidad_t remoto) {
	int id_q_envio = -1, id_q_recep = -1;

	if (local.proceso == entidad_t::CINE) {
		if (remoto.proceso == entidad_t::ADMIN) {
			id_q_envio = msg_queue_get(Q_CINE_ADMIN);
			id_q_recep = msg_queue_get(Q_ADMIN_CINE);
		}
		else if (remoto.proceso == entidad_t::CLIENTE) {
			id_q_envio = msg_queue_get(Q_CINE_CLI);
			id_q_recep = msg_queue_get(Q_CLI_CINE);
		}
	}
	else if (local.proceso == entidad_t::ADMIN && remoto.proceso == entidad_t::CINE) {
		id_q_envio = msg_queue_get(Q_ADMIN_CINE);
		id_q_recep = msg_queue_get(Q_CINE_ADMIN);
	}
	else if (local.proceso == entidad_t::CLIENTE && (remoto.proceso == entidad_t::CINE)) {
		id_q_envio = msg_queue_get(Q_CLI_CINE);
		id_q_recep = msg_queue_get(Q_CINE_CLI);
	}

	if (id_q_envio == -1 || id_q_recep == -1) {
		std::cerr << "Error al crear canal de comunicacion: " << strerror(errno) << std::endl;
		return NULL;
	}

    canal *c = (canal *) malloc(sizeof(canal));
	if (c != NULL) {
		c->local = local;
		c->remoto = remoto;
		c->id_queue_envio = id_q_envio;
		c->id_queue_recepcion = id_q_recep;
	}
	return c;
}

/* TENGO Q RESOLVER EL TEMA DE CÓMO SETEAR EL MSGID DESDE ACA (SIN QUE ME LO SETEEN LOS CALLER)
 * A PARTIR DE LA INFO Q DISPONGO (ENTIDADES ORIGEN Y DESTINO Y SUS PIDS) */
int canal_enviar(const canal *canal, mensaje_t msg) {
    printf("[%i] Enviado mensaje en cola %i con mtype %i\n", getpid(), canal->id_queue_envio, msg.mtype);
    return msg_queue_send(canal->id_queue_envio, &msg);
}

int canal_recibir(const canal *canal, mensaje_t &msg, long mtype) {
    printf("[%i] Esperando mensaje en cola %i\n", getpid(), canal->id_queue_recepcion);
    if (msg_queue_receive(canal->id_queue_recepcion, mtype, &msg)) {
        return msg.mtype;
    }
    perror("Error al recibir: ");
    return -1;
}

void canal_destruir(canal *canal) {
	free(canal);
}
