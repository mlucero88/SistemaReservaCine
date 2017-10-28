#ifndef COMMON_IPC_MSG_QUEUE_H_
#define COMMON_IPC_MSG_QUEUE_H_

#include "../operaciones.h"

// OJO: 0 no es valido para ftok!
typedef enum {
	Q_CINE_CLI_A = 1,	// socket_adapter_cli -> mom
	Q_CLI_CINE_A,		// mom -> socket_adapter_cli
	Q_CINE_CLI_B,		// cine -> socket_adapter_cine
	Q_CLI_CINE_B,		// socket_adapter_cine -> cine
	Q_ADMIN_CINE,		// admin -> cine
	Q_CINE_ADMIN,		// cine -> admin
	Q_CLI_MOM,			// cliente -> mom
	Q_MOM_CLI,			// mom -> cliente
	Q_ADMIN_CLI_A,            // admin -> cli (borrar?)
	Q_ADMIN_CLI_B            // admin -> cli (borrar?)
} msg_queue_direction;

/* Estructura de mensaje unico, usado por el sistema */
typedef struct {
    long mtype;
    int tipo; // cual de todas las operaciones es!
    union {
    	op_login_t login;
    	op_info_salas_t info_salas;
    	op_elegir_sala_t elegir_sala;
    	op_info_asientos_t info_asientos;
    	op_elegir_asientos_t elegir_asientos;
    	op_info_reserva_t info_reserva;
    	op_confirmar_reserva_t confirmar_reserva;
    	op_info_pago_t info_pago;
    	op_pagar_t pagar;
    	op_pago_ok_t pago_ok;
    	op_reserva_cancelada_t reserva_cancelada;
    	op_timeout_t timeout;
    	op_mom_init_t mom_init;
    	op_mom_init_reply_t mom_init_reply;
    	op_mom_destroy_t mom_destroy;
    } op;
} mensaje_t;

/* Usados solamente por proceso environment */
int msg_queue_create(msg_queue_direction n);

int msg_queue_destroy(int q_id);

/********************************************/

int msg_queue_get(msg_queue_direction n);

bool msg_queue_send(int q_id, const mensaje_t *mensaje);

bool msg_queue_receive(int q_id, long msg_type, mensaje_t *mensaje, int flags = 0);

#endif /* COMMON_IPC_MSG_QUEUE_H_ */
