#ifndef COMMON_IPC_MSG_QUEUE_H_
#define COMMON_IPC_MSG_QUEUE_H_


#include "../operaciones.h"

typedef enum {
	Q_CINE_CLI = 1, Q_CLI_CINE = 2, Q_ADMIN_CINE = 3, Q_CINE_ADMIN = 4, Q_ADMIN_CLI = 5
} msg_queue_direction;

/* Estructura de mensaje unico, usado por el sistema */
typedef struct {
	long mtype;
    int tipo; // cual de todas las operaciones es!
    operacion_t op;
} mensaje_t;

/* Usados solamente por proceso environment */
int msg_queue_create(msg_queue_direction n);
int msg_queue_destroy(int q_id);
/********************************************/

int msg_queue_get(msg_queue_direction n);
bool msg_queue_send(int q_id, const mensaje_t *mensaje);

bool msg_queue_receive(int q_id, long msg_type, mensaje_t *mensaje);

#endif /* COMMON_IPC_MSG_QUEUE_H_ */
