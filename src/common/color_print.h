#ifndef SRC_COMMON_COLOR_PRINT_H_
#define SRC_COMMON_COLOR_PRINT_H_

#include <cstdio>
#include <unistd.h>

#include "colors.h"
#include "constantes.h"

#define FPRINTF(file, color, fmt, ...) fprintf(file, color fmt KNRM, ##__VA_ARGS__); fflush(file)

static inline const char *strOpType(int opType) {
    switch (opType) {
        case LOGIN: {
            return "LOGIN";
        }
        case INFORMAR_SALAS: {
            return "INFORMAR_SALAS";
        }
        case ELEGIR_SALA: {
            return "ELEGIR_SALA";
        }
        case INFORMAR_ASIENTOS: {
            return "INFORMAR_ASIENTOS";
        }
        case ELEGIR_ASIENTOS: {
            return "ELEGIR_ASIENTOS";
        }
        case INFORMAR_RESERVA: {
            return "INFORMAR_RESERVA";
        }
        case CONFIRMAR_RESERVA: {
            return "CONFIRMAR_RESERVA";
        }
        case INFORMAR_PAGO: {
            return "INFORMAR_PAGO";
        }
        case PAGAR: {
            return "PAGAR";
        }
        case PAGO_OK: {
            return "PAGO_OK";
        }
        case RESERVA_CANCELADA: {
            return "RESERVA_CANCELADA";
        }
        case TIMEOUT: {
            return "TIMEOUT";
        }
        case MOM_INIT: {
            return "MOM_INIT";
        }
        case MOM_INIT_REPLY: {
            return "MOM_INIT_REPLY";
        }
        case MOM_DESTROY: {
            return "MOM_DESTROY";
        }
        default: {
            break;
        }
    }
    return "UNKOWN";
}

#endif /* SRC_COMMON_COLOR_PRINT_H_ */
