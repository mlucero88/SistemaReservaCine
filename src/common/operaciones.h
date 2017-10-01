#ifndef COMMON_OPERACIONES_H_
#define COMMON_OPERACIONES_H_

#include "constantes.h"

typedef union {

    struct {
        int cli_id;
    } login;

    struct {
        int asientos_por_sala[MAX_SALAS];
        int cant_salas;
    } info_salas;

    struct {
        int nro_sala;
    } elegir_sala;

    struct {
        int asiento_habilitado[MAX_ASIENTOS];
        int cant_asientos;
        int nro_sala;
    } info_asientos;

    struct {
        int asientos_elegidos[MAX_ASIENTOS_RESERVADOS];
        int cant_elegidos;
        int nro_sala;  // Por ahora, mando el numero de sala otra vez.
    } elegir_asientos;

    struct {
        int asientos_reservados[MAX_ASIENTOS_RESERVADOS];
        int cant_reservados;
    } info_reserva; /* Puede no ser la misma cantidad que los elegidos */

    struct {
        bool reserva_confirmada;
    } confirmar_reserva; /* Cliente acepta o no los asientos reservados */

    struct {
        unsigned short precio;
    } info_pago;

    struct {
        unsigned short pago;
    } pagar;

    struct {
        unsigned short pago_ok;
    } pago_ok;
    struct {
        int cli_id;
        int n_sala;
    } timeout;

} operacion_t;

#endif /* COMMON_OPERACIONES_H_ */
