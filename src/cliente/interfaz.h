#ifndef PROYECTO_INTERFAZ_H
#define PROYECTO_INTERFAZ_H

#include "../common/constantes.h"

#define RET_ERROR -1 // retorno si hay un error
#define RET_OK 0
#define RET_TIMEOUT 1 // retorno si hay timeout

typedef void *m_id;

m_id m_init(); // devuelve id

int m_login(m_id id, int cli_id);

int m_info_salas(m_id id, int asientos_por_sala[MAX_SALAS], int *cant_salas);

int m_asientos_sala(m_id id, int nro_sala, int asientos_habilitados[MAX_ASIENTOS],
                    int *cant_asientos);

int m_reservar_asientos(m_id id, int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos, int nro_sala,
                        int asientos_reservados[MAX_ASIENTOS_RESERVADOS], int *cant_reservados);

int m_confirmar_reserva(m_id id, bool aceptar, int *precio);

int m_pagar(m_id id, int precio);

#endif //PROYECTO_INTERFAZ_H
