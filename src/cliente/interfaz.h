#ifndef PROYECTO_INTERFAZ_H
#define PROYECTO_INTERFAZ_H

#include <functional>

#include "../common/constantes.h"
#include "../common/operaciones.h"

/* Posibles valores de m_errno */
#define RET_OK			 0
#define ERR_CLIID		-10	// cli_id invalido
#define ERR_QUEUEGET	-11	// Error en la obtencion de las colas
#define ERR_MSGSEND		-12	// Error en el envio de mensaje
#define ERR_MSGRECV		-13	// Error en la recepcion del mensaje
#define ERR_OPINVALID	-14	// La operacion no es valida en este momento (orden de operacion invalido)
#define ERR_TIMEOUT		-15	// retorno si hay timeout

/* Los errores se setean en m_errno */
extern int m_errno;

/* Funcion que retorna informacion sobre el codigo de error */
const char* m_str_error(int err);

/* Inicia el MOM cliente.
 *
 * @return
 * - cli_id en caso de m_errno == RET_OK
 * - basura en caso de error
 */
uuid_t m_init();

/* Destruye el MOM cliente.

 * @param cli_id: id retornado por m_init()
 *
 * @pre: Haber llamado a m_init()
 */
void m_dest(uuid_t cli_id);

/*
 * Loguea al cliente con el cine y carga los datos de las salas.
 *
 * @param cli_id: id retornado por m_init()
 *
 * @return
 * - informacion de las salas en caso de m_errno == RET_OK
 * - basura en caso de error
 *
 * @pre: Haber llamado a m_init() en el ultimo uso del api con este cli_id
 */
op_info_salas_t m_login(uuid_t cli_id);

/*
 * Selecciona una sala del cine y carga los asientos de las sala.

 * @param cli_id: id retornado por m_init()
 * @param nro_sala: el numero de sala que se desea seleccionar

 * @return
 * - informacion de los asientos en la sala en caso de m_errno == RET_OK
 * - basura en caso de error
 *
 * @pre: Haber llamado a m_login() en el ultimo uso del api con este cli_id
 */
op_info_asientos_t m_seleccionar_sala(uuid_t cli_id, int nro_sala);

/*
 * Intenta reservar los asienteos elegidos en la sala y carga la info de la reserva realizada.
 *
 * @param cli_id: id retornado por m_init()
 * @param asientos: ids de asientos elegidos
 * @param n_asientos: cantidad de asientos elegidos (<= MAX_ASIENTOS_RESERVADOS)
 *
 * @return
 * - informacion de la reserva en caso de m_errno == RET_OK
 * - basura en caso de error
 *
 * @pre: Haber llamado a m_seleccionar_sala() en el ultimo uso del api con este cli_id
 */
op_info_reserva_t m_seleccionar_asientos(uuid_t cli_id, int asientos[MAX_ASIENTOS_RESERVADOS], int n_asientos);

/*
 * Confirma o cancela la reserva y retorna la informacion de pago en caso de confirmarla.
 *
 * @param cli_id: id retornado por m_init()
 * @param aceptar: true para confirmar, false para cancelar
 *
 * @return
 * - precio de la reserva en caso de aceptar reserva y m_errno == RET_OK
 * - basura en caso cancelar reserva o de error
 *
 * @pre: Haber llamado a m_seleccionar_asientos() en el ultimo uso del api con este cli_id
 */
op_info_pago_t m_confirmar_reserva(uuid_t cli_id, bool aceptar);

/*
 * Realiza el pago de la reserva.
 *
 * @param cli_id: id retornado por m_init()
 * @param pago: dinero transferido (es una formalidad)
 *
 * @pre: Haber llamado a m_confirmar_reserva() en el ultimo uso del api con este cli_id
 */
void m_pagar(uuid_t cli_id, int pago);

/*
 * Registra una funcion de callback para el evento de notificacion de cambios en la sala.
 * Esta funcion se ejecuta en otro hilo de ejecucion.
 *
 * @param cli_id: id retornado por m_init()
 * @param handler: callback ejecutada asincronicamente al recibir evento de notificacion
 *
 * @pre: Haber llamado a m_init()
 */
void m_reg_cb_actualizacion_sala(uuid_t cli_id, std::function<void(const op_info_asientos_t&)> handler);

#endif //PROYECTO_INTERFAZ_H
