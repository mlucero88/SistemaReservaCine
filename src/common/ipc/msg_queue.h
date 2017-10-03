#ifndef COMMON_IPC_MSG_QUEUE_H_
#define COMMON_IPC_MSG_QUEUE_H_

#include "../operaciones.h"

typedef enum {
    Q_CINE_CLI = 1, Q_CLI_CINE, Q_ADMIN_CINE, Q_CINE_ADMIN, Q_ADMIN_CLI, Q_CLI_MOM, Q_MOM_CLI	// Para CINE y ADMIN, el CLI en realidad es el MOM, pero ellos no saben
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
