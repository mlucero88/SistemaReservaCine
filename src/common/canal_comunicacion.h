#ifndef COMMON_CANAL_COMUNICACION_H_
#define COMMON_CANAL_COMUNICACION_H_

#include "entidad.h"
#include "operaciones.h"

/* Abstraccion de un canal de comunicacion. En principio se implementa usando una msg_quue de ida y una msg_queue de vuelta */

struct canal_comunicacion;

canal_comunicacion* canal_comunicacion_crear(entidad_t local, entidad_t remoto);

bool canal_comunicacion_enviar(const canal_comunicacion *canal, const operacion_t &op);

bool canal_comunicacion_recibir(const canal_comunicacion *canal, operacion_t &op);

void canal_comunicacion_destruir(canal_comunicacion* canal);

#endif /* COMMON_CANAL_COMUNICACION_H_ */
