#ifndef COMMON_CANAL_COMUNICACION_H_
#define COMMON_CANAL_COMUNICACION_H_

#include "entidad.h"
#include "operaciones.h"
#include "ipc/msg_queue.h"
/* Abstraccion de un canal de comunicacion. En principio se implementa usando una msg_quue de ida y una msg_queue de vuelta */

struct canal;

canal *canal_crear(entidad_t local, entidad_t remoto);

int canal_enviar(const canal *canal, mensaje_t msg);

int canal_recibir(const canal *canal, mensaje_t &msg, long mtype);

void canal_destruir(canal *canal);

#endif /* COMMON_CANAL_COMUNICACION_H_ */
