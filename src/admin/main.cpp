#include <iostream>
#include <cstring>
#include "../common/canal.h"
#include "../common/operaciones.h"
#include "../common/ipc/msg_queue.h"

void informar_asientos(mensaje_t *m, canal *canal_cine_admin, int cine_id, int asientos_salas[MAX_SALAS][MAX_ASIENTOS],
                       int n_asientos_salas[MAX_SALAS]) {
    int nro_sala = m->operacion.elegir_sala.nro_sala;
    memcpy(m->operacion.informar_asientos.asiento_habilitado, asientos_salas[nro_sala], MAX_ASIENTOS * sizeof(int));
    m->operacion.informar_asientos.cantidad_asientos = n_asientos_salas[nro_sala];
    m->tipo = INFORMAR_ASIENTOS;
    m->operacion.informar_asientos.nro_sala = nro_sala;
    printf("[ADMIN] Enviando INFORMAR_ASIENTOS\n");
    canal_enviar(canal_cine_admin, *m);
}

void informar_salas(mensaje_t *m, canal *canal_cine_admin, int cine_id, int n_salas, int n_asientos_salas[MAX_SALAS]) {
    memcpy(m->operacion.informar_salas.asientos_por_sala, n_asientos_salas, MAX_SALAS * sizeof(int));
    m->operacion.informar_salas.cantidad_salas = n_salas;
    printf("[ADMIN] Enviando INFORMAR_SALAS\n");
    canal_enviar(canal_cine_admin, *m);
}

void informar_reserva(mensaje_t *m, canal *canal_cine_admin, int cine_id, int asientos_salas[MAX_SALAS][MAX_ASIENTOS],
                      int n_asientos_salas[MAX_SALAS]) {
    mensaje_t msg;
    msg.tipo = INFORMAR_RESERVA;
    msg.mtype = cine_id;

    int nro_asientos_pedidos = m->operacion.elegir_asientos.cantidad_elegidos;
    int asientos_reservados = 0;
    int nro_sala = m->operacion.elegir_asientos.nro_sala; // ***
    printf("[ADMIN] Veo si se pueden reservar %i asients en la sala %i\n", nro_asientos_pedidos, nro_sala + 1);
    for (int i = 0; i < nro_asientos_pedidos; i++) {
        int nro_asiento = m->operacion.elegir_asientos.asientos_elegidos[i];
        if (asientos_salas[nro_sala][nro_asiento] == DISPONIBLE) {
            msg.operacion.informar_reserva.asientos_reservados[i] = 1;
            asientos_salas[nro_sala][nro_asiento] = RESERVADO;
            asientos_reservados++;
            printf("[ADMIN] Un asiento reservado: %i\n", nro_asiento);
        } else {
            msg.operacion.informar_reserva.asientos_reservados[i] = 0;
        }
    }
    msg.operacion.informar_reserva.cantidad_reservados = asientos_reservados;

    printf("[ADMIN] Enviando INFORMAR_RESERVA\n");
    canal_enviar(canal_cine_admin, msg);
}

void informar_pago(mensaje_t *m, canal *canal_cine_admin, int cine_id) {
    mensaje_t msg;
    msg.tipo = INFORMAR_PAGO;
    msg.mtype = cine_id;
    msg.operacion.informar_pago.precio = 500;
    printf("[ADMIN] Enviando INFORMAR_PAGO\n");
    canal_enviar(canal_cine_admin, msg);
}

void pago_ok(mensaje_t *m, canal *canal_cine_admin, int cine_id) {
    mensaje_t msg;
    msg.tipo = PAGO_OK;
    msg.mtype = cine_id;
    msg.operacion.pago_ok.pago_ok = 500;
    printf("[ADMIN] Enviando PAGO_OK\n");
    canal_enviar(canal_cine_admin, msg);
}

void cargar_datos(int n_salas, int n_asientos_salas[MAX_SALAS], int asientos_salas[MAX_SALAS][MAX_ASIENTOS]) {

    for (int i = 0; i < n_salas; i++) {
        int n_asientos = rand() % MAX_ASIENTOS;
        n_asientos_salas[i] = n_asientos;
        for (int j = 0; j < n_asientos; j++) {
            asientos_salas[i][j] = DISPONIBLE;
        }
        for (int j = n_asientos; j < MAX_ASIENTOS; j++) {
            asientos_salas[i][j] = NO_DISPONIBLE;
        }
    }
    for (int i = n_salas; i < MAX_SALAS; i++) {
        n_asientos_salas[i] = 0;
        for (int j = 0; j < MAX_ASIENTOS; j++) {
            asientos_salas[i][j] = NO_DISPONIBLE;
        }
    }
}

int main(int argc, char *argv[]) {

    int n_salas = rand() % MAX_SALAS; // Cantidad de salas que hay en total
    printf("Cantidad de salas: %i\n", n_salas);
    int n_asientos_salas[MAX_SALAS]; // Cantidad de asientos que hay en cada sala
    int asientos_salas[MAX_SALAS][MAX_ASIENTOS]; // Asientos disponibles/ocupados en cada sala

    cargar_datos(n_salas, n_asientos_salas, asientos_salas);

    for (int i = 0; i < MAX_SALAS; i++) {
        printf("SALA %i -> %i\n", i + 1, n_asientos_salas[i]);
        for (int j = 0; j < MAX_ASIENTOS; j++) {
            printf("%c", asientos_salas[i][j] == DISPONIBLE ? 'O' : 'X');
        }
        printf("\n");
    }

    printf("Iniciado el admin\n");
    entidad_t cine = {.proceso = entidad_t::CINE, .pid = getpid()};
    entidad_t admin = {.proceso = entidad_t::ADMIN, .pid = -1};

    canal *canal_cine_admin = canal_crear(admin, cine);
    if (canal_cine_admin == NULL) {
        std::cerr << "Error al crear canal de comunicacion entre cine y cliente" << std::endl;
        exit(1);
    }

    while (true) {
        mensaje_t msg;
        printf("[ADMIN] Esperando mensaje\n");
        long cine_id = canal_recibir(canal_cine_admin, msg, 0); // Agarro cualquier mensaje que me hayan mandado
        printf("[ADMIN] Mensaje recibido\n");
        printf("[ADMIN] Recibí mensaje con tipo %i y mtype %i\n", msg.tipo, msg.mtype);
        if (msg.tipo == INFORMAR_SALAS) {
            printf("[ADMIN] INFORMAR_SALAS para %i\n", cine_id);
            informar_salas(&msg, canal_cine_admin, cine_id, n_salas, n_asientos_salas);

        }

        if (msg.tipo == ELEGIR_SALA) {
            printf("[ADMIN] INFORMAR_ASIENTOS para %i\n", cine_id);
            informar_asientos(&msg, canal_cine_admin, cine_id, asientos_salas, n_asientos_salas);

        }

        if (msg.tipo == ELEGIR_ASIENTOS) {
            printf("[ADMIN] INFORMAR_RESERVA para %i\n", cine_id);
            informar_reserva(&msg, canal_cine_admin, cine_id, asientos_salas, n_asientos_salas);

        }

        if (msg.tipo == CONFIRMAR_RESERVA) {
            printf("[ADMIN] INFORMAR_PAGO para %i\n", cine_id);
            informar_pago(&msg, canal_cine_admin, cine_id);

        }

        if (msg.tipo == PAGAR) {
            pago_ok(&msg, canal_cine_admin, cine_id);

        }


    }

}