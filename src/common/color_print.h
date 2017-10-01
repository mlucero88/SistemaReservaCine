#ifndef SRC_COMMON_COLOR_PRINT_H_
#define SRC_COMMON_COLOR_PRINT_H_

#include <cstdio>
#include <unistd.h>

#include "constantes.h"

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

#define FPRINTF(file, color, fmt, ...) fprintf(file, color "[%i] " fmt RESET "\n", getpid(), ##__VA_ARGS__); fflush(file)

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
        case TIMEOUT: {
            return "TIMEOUT";
        }
        default: {
            break;
        }
    }
    return "UNKOWN";
}

#endif /* SRC_COMMON_COLOR_PRINT_H_ */
