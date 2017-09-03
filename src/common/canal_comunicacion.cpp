#include <cerrno>
#include <cstring>
#include <iostream>

#include "ipc/msg_queue.h"
#include "canal_comunicacion.h"

#define LOGIN_MSG_TYPE 1 /* pid = 1 esel init, asi que no hay problema */

struct canal_comunicacion {
	entidad_t local;
	entidad_t remoto;
	int id_queue_envio;
	int id_queue_recepcion;
};

/* Otra opcion es hacer una funcion para crear un canal especifico
 * Ej: canal_comunicacion_crear_cine_cliente()
 * Pero como está ahora es mas versatil, aunque sean feos los ifs */
canal_comunicacion* canal_comunicacion_crear(entidad_t local, entidad_t remoto) {
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

	canal_comunicacion *c = (canal_comunicacion*) malloc(sizeof(canal_comunicacion));
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
bool canal_comunicacion_enviar(const canal_comunicacion *canal, const operacion_t &op) {
	mensaje_t m;
	m.operacion = op;	/* Anda bien esta copia, o tengo q hacer memcpy por los arrays q tengo adentro? No me acuerdo */

	if(op.tipo == operacion_t::LOGIN) {
		/* Caso particular donde el msgtype no es un pid */
		m.mtype = LOGIN_MSG_TYPE;
	}
	else {
		// HACER LOGICA PARA DETERMINAR EL MTYPE
		m.mtype = 0;
	}

	return msg_queue_send(canal->id_queue_envio, &m);
}

bool canal_comunicacion_recibir(const canal_comunicacion *canal, operacion_t &op) {
	mensaje_t m;
	// HACER LOGICA PARA DETERMINAR EL MTYPE
	long mtype = 0;

	if(msg_queue_recieve(canal->id_queue_recepcion, mtype, &m)) {
		op = m.operacion; /* Anda bien esta copia, o tengo q hacer memcpy por los arrays q tengo adentro? No me acuerdo */
		return true;
	}
	return false;
}

void canal_comunicacion_destruir(canal_comunicacion* canal) {
	free(canal);
}
