#ifndef COMMON_OPERACIONES_H_
#define COMMON_OPERACIONES_H_

#include "constantes.h"

struct op_login_t {
	int cli_id;
};

struct op_info_salas_t {
	int asientos_por_sala[MAX_SALAS];
	int cant_salas;
};

struct op_elegir_sala_t {
	int nro_sala;
};

struct op_info_asientos_t {
	int asiento_habilitado[MAX_ASIENTOS];
	int cant_asientos;
	int nro_sala;
};

struct op_elegir_asientos_t {
	int asientos_elegidos[MAX_ASIENTOS_RESERVADOS];
	int cant_elegidos;
	int nro_sala;
};

/* Puede no ser la misma cantidad que los elegidos */
struct op_info_reserva_t {
	int asientos_reservados[MAX_ASIENTOS_RESERVADOS];
	int cant_reservados;
};

struct op_confirmar_reserva_t {
	bool reserva_confirmada;
	int nro_sala;
};

struct op_info_pago_t {
	unsigned short precio;
};

struct op_pagar_t {
	unsigned short pago;
};

struct op_pago_ok_t {
};

struct op_reserva_cancelada_t {
};

struct op_timeout_t {
	int cli_id;
	int n_sala;
};

#endif /* COMMON_OPERACIONES_H_ */
