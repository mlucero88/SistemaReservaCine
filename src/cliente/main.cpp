#include <random>
#include <cerrno>
#include <cstring>
#include <iostream>

#include "common/canal_comunicacion.h"

static nro_sala_t elegir_sala_rand(const nro_asiento_t *asientos_por_sala, nro_sala_t cantidad_salas) {
	nro_sala_t sala;
	do {
		sala = rand() % cantidad_salas;
	} while (asientos_por_sala[sala] < 1);
	return sala;
}

static nro_asiento_t elegir_asientos_rand(const bool *asiento_habilitado, nro_asiento_t cantidad_asientos, nro_asiento_t *asientos_elegidos) {
	nro_asiento_t rounds = 1 + (rand() % MAX_ASIENTOS_RESERVADOS);
	nro_asiento_t id_asiento;
	int i = 0;
	int intentos = 20; /* Corte por si no hay suficientes asientos */

	while(rounds > 0 && intentos > 0) {
		id_asiento = rand() % cantidad_asientos;
		if(asiento_habilitado[id_asiento]) {
			/* Reviso que no lo haya elegido en un round anterior */
			bool ya_elegido = false;
			for(int j = 0; j < i; ++j) {
				if(id_asiento == asientos_elegidos[j]) {
					ya_elegido = true;
					break;
				}
			}
			if(!ya_elegido) {
				asientos_elegidos[i++] = id_asiento;
				--rounds;
			}
		}
		--intentos;
	}

	return i;
}

int main() {
	entidad_t cliente = { .proceso = entidad_t::CLIENTE, .pid = getpid() };
	entidad_t cine = { .proceso = entidad_t::CINE, .pid = -1 };

	canal_comunicacion* canal_cli_cine = canal_comunicacion_crear(cliente, cine);
	if (canal_cli_cine == NULL) {
		std::cerr << "Error al crear canal de comunicacion entre cliente y cine" << std::endl;
		exit(1);
	}

	srand(time(NULL));
	operacion_t op;

	/* Envio LOGIN */
	op.tipo = operacion_t::LOGIN;
	op.login.cli_id = cliente.pid;
	if (!canal_comunicacion_enviar(canal_cli_cine, op)) {
		std::cerr << "Error al enviar mensaje de LOGIN: " << strerror(errno) << std::endl;
		exit(1);
	}

	/* Recibo informacion salas */
	if (!canal_comunicacion_recibir(canal_cli_cine, op) || op.tipo != operacion_t::INFORMAR_SALAS) {
		std::cerr << "Error al recibir mensaje de INFORMAR_SALAS" << std::endl;
		exit(1);
	}

	/* Envio nro sala elegida */
	op.tipo = operacion_t::ELEGIR_SALA;
	op.elegir_sala.nro_sala = elegir_sala_rand(op.informar_salas.asientos_por_sala, op.informar_salas.cantidad_salas);
	if (!canal_comunicacion_enviar(canal_cli_cine, op)) {
		std::cerr << "Error al enviar mensaje de ELEGIR_SALA: " << strerror(errno) << std::endl;
		exit(1);
	}

	/* Recibo informacion asientos de la sala */
	if (!canal_comunicacion_recibir(canal_cli_cine, op) || op.tipo != operacion_t::INFORMAR_ASIENTOS) {
		std::cerr << "Error al recibir mensaje de INFORMAR_ASIENTOS" << std::endl;
		exit(1);
	}

	/* Envio asientos elegidos */
	nro_asiento_t asientos[MAX_ASIENTOS_RESERVADOS];
	nro_asiento_t cantidad = elegir_asientos_rand(op.informar_asientos.asiento_habilitado, op.informar_asientos.cantidad_asientos, asientos);
	op.tipo = operacion_t::ELEGIR_ASIENTOS;
	memcpy(op.elegir_asientos.asientos_elegidos, asientos, sizeof(nro_asiento_t) * cantidad);
	op.elegir_asientos.cantidad_elegidos = cantidad;
	if (!canal_comunicacion_enviar(canal_cli_cine, op)) {
		std::cerr << "Error al enviar mensaje de LOGIN: " << strerror(errno) << std::endl;
		exit(1);
	}

	/* Recibo informacion de la reserva */
	if (!canal_comunicacion_recibir(canal_cli_cine, op) || op.tipo != operacion_t::INFORMAR_RESERVA) {
		std::cerr << "Error al recibir mensaje de INFORMAR_RESERVA" << std::endl;
		exit(1);
	}

	/* Envio confirmacion de reserva */
	op.tipo = operacion_t::CONFIRMAR_RESERVA;
	op.confirmar_reserva.reserva_confirmada = ((rand() % 10) > 0);	/* 9/10 de ser confirmada */
	if (!canal_comunicacion_enviar(canal_cli_cine, op)) {
		std::cerr << "Error al enviar mensaje de CONFIRMAR_RESERVA: " << strerror(errno) << std::endl;
		exit(1);
	}

	/* Recibo informacion de pago */
	if (!canal_comunicacion_recibir(canal_cli_cine, op) || op.tipo != operacion_t::INFORMAR_PAGO) {
		std::cerr << "Error al recibir mensaje de INFORMAR_PAGO" << std::endl;
		exit(1);
	}

	/* Envio pago */
	op.tipo = operacion_t::PAGAR;
	op.pagar.pago = op.informar_pago.precio;
	if (!canal_comunicacion_enviar(canal_cli_cine, op)) {
		std::cerr << "Error al enviar mensaje de PAGAR: " << strerror(errno) << std::endl;
		exit(1);
	}

	canal_comunicacion_destruir(canal_cli_cine);

	return 0;
}
