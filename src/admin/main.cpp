#include <iostream>
#include "../common/canal.h"
#include "../common/operaciones.h"
#include "../common/ipc/msg_queue.h"

void informar_asientos(mensaje_t *m, canal *canal_cine_admin, int cine_id) {
    int *a = m->operacion.informar_asientos.asiento_habilitado;
    a[0] = 1;
    a[1] = 1;
    a[2] = 1;
    a[3] = 1;
    a[4] = 1;
    m->operacion.informar_asientos.cantidad_asientos = 5;
    m->tipo = INFORMAR_ASIENTOS;
    printf("[ADMIN] Enviando INFORMAR_ASIENTOS\n");
    canal_enviar(canal_cine_admin, *m);
}

void informar_salas(mensaje_t *m, canal *canal_cine_admin, int cine_id) {
    int *a = m->operacion.informar_salas.asientos_por_sala;
    a[0] = 5;
    a[1] = 5;
    a[2] = 5;
    a[3] = 5;
    a[4] = 5;
    m->operacion.informar_salas.cantidad_salas = 5;
    printf("[ADMIN] Enviando INFORMAR_SALAS\n");
    canal_enviar(canal_cine_admin, *m);
}

void informar_reserva(mensaje_t *m, canal *canal_cine_admin, int cine_id) {
    mensaje_t msg;
    msg.tipo = INFORMAR_RESERVA;
    msg.mtype = cine_id;
    int *b = msg.operacion.informar_reserva.asientos_reservados;
    b[0] = 5;
    b[1] = 5;
    b[2] = 5;
    b[3] = 5;
    b[4] = 5;
    msg.operacion.informar_reserva.cantidad_reservados = 5;
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

int main(int argc, char *argv[]) {
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
            informar_salas(&msg, canal_cine_admin, cine_id);

        }

        if (msg.tipo == ELEGIR_SALA) {
            printf("[ADMIN] INFORMAR_ASIENTOS para %i\n", cine_id);
            informar_asientos(&msg, canal_cine_admin, cine_id);

        }

        if (msg.tipo == ELEGIR_ASIENTOS) {
            printf("[ADMIN] INFORMAR_RESERVA para %i\n", cine_id);
            informar_reserva(&msg, canal_cine_admin, cine_id);

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
