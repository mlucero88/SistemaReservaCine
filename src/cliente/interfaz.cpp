#include <cstdlib>
#include <cstring>

#include "interfaz.h"

#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"

//#define DEBUG_

#ifdef DEBUG_
/* Los logs a causa de error se hacen desde el cliente que consume el api */
#define INT_PRINTF(fmt, ...) FPRINTF(stdout, KBLU, fmt, ##__VA_ARGS__)
#else
#define INT_PRINTF(fmt, ...)
#endif

enum CliLastState {
	INIT, CINE_LOGIN, SELECCION_SALA, SELECCION_ASIENTOS, CONFIRMACION_RESERVA, PAGO, EXPIRADO, CANCELADO
};

int m_errno;

/* Como la relacion api <-> cliente es 1 a 1 (al menos ahora), puedo usar static, si no
 * tengo que mapear estados a cada cli_id con un map */
static CliLastState cli_state;
static int _sala;

/* Colas con el mismo id para todos los procesos cliente que existan */
static int q_mom_snd;
static int q_mom_rcv;


int enviar_msj(mensaje_t &msg) {
	return msg_queue_send(q_mom_snd, &msg) ? RET_OK : ERR_MSGSEND;
}

int recibir_msj(long cli_id, mensaje_t &msg, int tipo) {
    INT_PRINTF("Esperando mensaje\n");
	if (!msg_queue_receive(q_mom_rcv, cli_id, &msg)) {
		return ERR_MSGRECV;
	}
    INT_PRINTF("Mensaje recibido\n");

	if (msg.tipo != tipo) {
		if (msg.tipo == TIMEOUT) {
			cli_state = EXPIRADO;
			return ERR_TIMEOUT;
		}
		else {
            INT_PRINTF("Quería %s, recibí %s\n", strOpType(tipo), strOpType(msg.tipo));
			INT_PRINTF("No deberia entrar nunca aca!!\n");
			std::abort();
		}
	}
	return RET_OK;
}

m_id m_init() {
    q_mom_snd = msg_queue_get(Q_CLI_MOM);
    q_mom_rcv = msg_queue_get(Q_MOM_CLI);

	if (q_mom_snd == -1 || q_mom_rcv == -1) {
		m_errno = ERR_QUEUEGET;
	}

	/* todo crear id de otra forma (q no sea 1 xq es para el mtype de LOGIN) */
	m_id cli_id = getpid();

	cli_state = INIT;
	m_errno = RET_OK;
	return cli_id;
}

void m_dest(m_id cli_id) {
	/* todo informar al mom para q saque el id de su tabla */
	m_errno = RET_OK;
}

op_info_salas_t m_login(m_id cli_id) {
	if(cli_state != INIT) {
        m_errno = ERR_OPINVALID;
        return op_info_salas_t {};
	}

	int ret;
	mensaje_t msg = { .mtype = LOGIN_MSG_TYPE, .tipo = LOGIN };
	msg.op.login.cli_id = cli_id;

	if((ret = enviar_msj(msg)) == RET_OK) {
        INT_PRINTF("Se envió LOGIN ok\n");
		if((ret = recibir_msj(cli_id, msg, INFORMAR_SALAS)) == RET_OK) {
			cli_state = CINE_LOGIN;
			m_errno = RET_OK;
			return msg.op.info_salas;
		}
	}

	m_errno = ret;
	return op_info_salas_t {};
}

op_info_asientos_t m_seleccionar_sala(m_id cli_id, int nro_sala) {
	if(cli_state != CINE_LOGIN) {
        m_errno = ERR_OPINVALID;
        return op_info_asientos_t {};
	}

	int ret;
	mensaje_t msg = { .mtype = cli_id, .tipo = ELEGIR_SALA };
	msg.op.elegir_sala.nro_sala = nro_sala;
    INT_PRINTF("Mando elegir sala con nro %i\n", nro_sala);
	if((ret = enviar_msj(msg)) == RET_OK) {
		if((ret = recibir_msj(cli_id, msg, INFORMAR_ASIENTOS)) == RET_OK) {
			cli_state = SELECCION_SALA;
			_sala = nro_sala;
			m_errno = RET_OK;
			return msg.op.info_asientos;
		}
	}

	m_errno = ret;
	return op_info_asientos_t {};
}

op_info_reserva_t m_seleccionar_asientos(m_id cli_id, int asientos[MAX_ASIENTOS_RESERVADOS], int n_asientos) {
	if(cli_state != SELECCION_SALA) {
        m_errno = ERR_OPINVALID;
        return op_info_reserva_t {};
	}

	int ret;
	mensaje_t msg = { .mtype = cli_id, .tipo = ELEGIR_ASIENTOS };
	memcpy(msg.op.elegir_asientos.asientos_elegidos, asientos, sizeof(int) * n_asientos);
    for (int i = 0; i < MAX_ASIENTOS_RESERVADOS; i++) {
        INT_PRINTF("ASIENTO %i\n", asientos[i]);
    }
	msg.op.elegir_asientos.cant_elegidos = n_asientos;
    msg.op.elegir_asientos.nro_sala = _sala;

	if((ret = enviar_msj(msg)) == RET_OK) {
        if ((ret = recibir_msj(cli_id, msg, INFORMAR_RESERVA)) == RET_OK) {
			cli_state  = SELECCION_ASIENTOS;
			m_errno = RET_OK;
			return msg.op.info_reserva;
		}
	}

	m_errno = ret;
	return op_info_reserva_t {};
}

op_info_pago_t m_confirmar_reserva(m_id cli_id, bool aceptar) {
	if(cli_state != SELECCION_ASIENTOS) {
        m_errno = ERR_OPINVALID;
        return op_info_pago_t {};
	}

	int ret;
	mensaje_t msg = { .mtype = cli_id, .tipo = CONFIRMAR_RESERVA };
	msg.op.confirmar_reserva.reserva_confirmada = aceptar;
	msg.op.confirmar_reserva.nro_sala = _sala;

	if((ret = enviar_msj(msg)) == RET_OK) {
		int tipo = aceptar ? INFORMAR_PAGO : RESERVA_CANCELADA;
		if((ret = recibir_msj(cli_id, msg, tipo)) == RET_OK) {
			m_errno = RET_OK;

			if(aceptar) {
				cli_state = CONFIRMACION_RESERVA;
				return msg.op.info_pago;
			}
			else {
				cli_state = CANCELADO;
				return op_info_pago_t {};
			}
		}
	}

	m_errno = ret;
	return op_info_pago_t {};
}

void m_pagar(m_id cli_id, int pago) {
	if(cli_state != CONFIRMACION_RESERVA) {
        m_errno = ERR_OPINVALID;
        return;
	}

	int ret;
	mensaje_t msg = { .mtype = cli_id, .tipo = PAGAR };
	msg.op.pagar.pago = pago;

	if((ret = enviar_msj(msg)) == RET_OK) {
		if((ret = recibir_msj(cli_id, msg, PAGO_OK)) == RET_OK) {
			cli_state = PAGO;
			m_errno = RET_OK;
			return;
		}
	}

	m_errno = ret;
}

const char* m_str_error(int err) {
	switch (err) {
	case ERR_CLIID: {
		return "Invalid client id";
	}
	case ERR_QUEUEGET: {
		return "Cannot get queue";
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
	case ERR_TIMEOUT: {
		return "Timeout reached!";
	}
	default: {
		break;
	}
	}
	return "";
}
