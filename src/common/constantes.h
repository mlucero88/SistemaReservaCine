#ifndef COMMON_CONSTANTES_H_
#define COMMON_CONSTANTES_H_

#include <cstdint>

/* Las salas del cine se numeran de 0 a X de forma secuencial, con 0 < X < MAX_SALAS */
/* Los asientos en una sala se numeran de 0 a Y de forma secuencial, con 0 < Y < MAX_ASIENTOS */

#define LOGIN_MSG_TYPE 4
#define LOGIN 1
#define INFORMAR_SALAS 2
#define ELEGIR_SALA 3
#define INFORMAR_ASIENTOS 4
#define ELEGIR_ASIENTOS 5
#define INFORMAR_RESERVA 6
#define CONFIRMAR_RESERVA 7
#define INFORMAR_PAGO 8
#define PAGAR 9
#define PAGO_OK 10



namespace {
const unsigned short MAX_SALAS = 64;
const unsigned short MAX_ASIENTOS = 128;
const unsigned short MAX_ASIENTOS_RESERVADOS = 8;
};



#endif /* COMMON_CONSTANTES_H_ */
