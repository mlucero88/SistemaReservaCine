#ifndef COMMON_CONSTANTES_H_
#define COMMON_CONSTANTES_H_

/* Las salas del cine se numeran de 0 a X de forma secuencial, con 0 < X < MAX_SALAS */
/* Los asientos en una sala se numeran de 0 a Y de forma secuencial, con 0 < Y < MAX_ASIENTOS */

//#define LOGIN_MSG_TYPE 4
#define LOGIN_MSG_TYPE 1

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
#define RESERVA_CANCELADA 11
#define TIMEOUT 12
#define MOM_INIT 13
#define MOM_INIT_REPLY 14
#define MOM_DESTROY 15
#define NOTIFICAR_CAMBIOS 16

#define NO_DISPONIBLE (-1) // El asiento no existe en la sala
#define DISPONIBLE 0 // El asiento se puede reservar
#define RESERVADO 1 // El asiento ya está reservado

#define TIMEOUT_VAL 60 // Timeout de la sesión de un cliente (lo subo xq no me da tiempo a probar)

#define MAX_SALAS 5
#define MAX_ASIENTOS 15
#define MAX_ASIENTOS_RESERVADOS 3

#define MAX_CLIENTES 20 // Máxima cantidad de clientes que pueden estar en una sala al mismo tiempo

typedef long uuid_t;

#endif /* COMMON_CONSTANTES_H_ */
