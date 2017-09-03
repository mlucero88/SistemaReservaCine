#ifndef COMMON_CONSTANTES_H_
#define COMMON_CONSTANTES_H_

#include <cstdint>

/* Las salas del cine se numeran de 0 a X de forma secuencial, con 0 < X < MAX_SALAS */
/* Los asientos en una sala se numeran de 0 a Y de forma secuencial, con 0 < Y < MAX_ASIENTOS */

namespace {
const unsigned short MAX_SALAS = 64;
const unsigned short MAX_ASIENTOS = 128;
const unsigned short MAX_ASIENTOS_RESERVADOS = 8;
}

typedef std::uint8_t nro_sala_t;
typedef unsigned short nro_asiento_t;

#endif /* COMMON_CONSTANTES_H_ */
