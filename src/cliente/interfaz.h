#ifndef PROYECTO_INTERFAZ_H
#define PROYECTO_INTERFAZ_H

#include "../common/constantes.h"

int m_init(); // devuelve id
int m_login(int id, int cli_id);

int m_info_salas(int id, int info_salas[MAX_SALAS], int *cant_salas);

int m_asientos_sala(int id, int nro_sala, int asientos_habilitados[MAX_ASIENTOS],
                    int *cant_asientos); // devuelve la info de los asientos de esta sala
int m_reservar_asientos(int id, int asientos[MAX_ASIENTOS_RESERVADOS], int cant_asientos, int sala,
                        int asientos_reservados[MAX_ASIENTOS_RESERVADOS], int *cant_reservados);

int m_confirmar_reserva(int id, bool aceptar, int *precio); // devuelve el precio a pagar
int m_pagar(int id);

#endif //PROYECTO_INTERFAZ_H
