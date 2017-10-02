#ifndef PROYECTO_INTERFAZ_H
#define PROYECTO_INTERFAZ_H

#include "../common/constantes.h"

/* Posibles valores de m_errno */
#define RET_OK			 0
#define ERR_MOMID		-1	// momId invalido
#define ERR_MSGSEND		-2	// Error en el envio de mensaje
#define ERR_MSGRECV		-3	// Error en la recepcion del mensaje
#define ERR_OPINVALID	-4	// La operacion no es valida en este momento (orden de operacion invalido)
#define ERR_SALAINVALID	-5	// El numero de sala es invalido
#define ERR_TIMEOUT		-6	// retorno si hay timeout

extern int m_errno;

/* Funcion que retorna informacion sobre el codigo de error */
const char* m_str_error(int err);

typedef void *m_id;	// todo buscar como generar el uuid

/* Los errores se setean en m_errno */
//todo documentar mejor

/* Retorna el id */
m_id m_init();

void m_dest(m_id id);

void m_login(m_id id, int cli_id);

/* Retorna cantidad de salas */
int m_info_salas(m_id id, int asientos_por_sala[MAX_SALAS]);

/* Retorna cantidad de asientos */
int m_asientos_sala(m_id id, int nro_sala, int asientos_habilitados[MAX_ASIENTOS]);

/* Retorna cantidad de asientos reservados */
int m_reservar_asientos(m_id id, int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos, int nro_sala,
                        int asientos_reservados[MAX_ASIENTOS_RESERVADOS]);

/* Retorna el precio. Si aceptar es "false", solo se envia el mensaje para avisar q se cancela, y no se espera recibir nada (esto es para saber desp para la implementacion) */
int m_confirmar_reserva(m_id id, bool aceptar);

void m_pagar(m_id id, int precio);


#endif //PROYECTO_INTERFAZ_H
