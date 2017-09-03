#ifndef COMMON_OPERACIONES_H_
#define COMMON_OPERACIONES_H_

#include "constantes.h"

typedef union {
	enum {
		LOGIN, INFORMAR_SALAS, ELEGIR_SALA, INFORMAR_ASIENTOS, ELEGIR_ASIENTOS, INFORMAR_RESERVA, CONFIRMAR_RESERVA, INFORMAR_PAGO, PAGAR
	} tipo;

	struct {
		int cli_id;
	} login;

	struct {
		nro_asiento_t asientos_por_sala[MAX_SALAS];
		nro_sala_t cantidad_salas;
	} informar_salas;

	struct {
		nro_sala_t nro_sala;
	} elegir_sala;

	struct {
		bool asiento_habilitado[MAX_ASIENTOS];
		nro_asiento_t cantidad_asientos;
	} informar_asientos;

	struct {
		nro_asiento_t asientos_elegidos[MAX_ASIENTOS_RESERVADOS];
		nro_asiento_t cantidad_elegidos;
	} elegir_asientos;

	struct {
		nro_asiento_t asientos_reservados[MAX_ASIENTOS_RESERVADOS];
		nro_asiento_t cantidad_reservados;
	} informar_reserva; /* Puede no ser la misma cantidad que los elegidos */

	struct {
		bool reserva_confirmada;
	} confirmar_reserva; /* Cliente acepta o no los asientos reservados */

	struct {
		unsigned short precio;
	} informar_pago;

	struct {
		unsigned short pago;
	} pagar;

} operacion_t;

#endif /* COMMON_OPERACIONES_H_ */
