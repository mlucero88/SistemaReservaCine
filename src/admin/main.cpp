#include <iostream>
#include <cstring>
#include "../common/canal.h"
#include "../common/colors.h"

void mostrar_asientos(int n_sala, int asientos_salas[MAX_SALAS][MAX_ASIENTOS]) {
    printf("\nHay %i asientos en total en la sala\n", n_sala + 1);
    for (int j = 0; j < MAX_ASIENTOS; j++) {
        printf("%c\n", asientos_salas[n_sala][j] == DISPONIBLE ? 'O' : 'X');
    }
    printf("\n");
}


void informar_asientos(mensaje_t *m, canal *canal_cine_admin, int cine_id, int asientos_salas[MAX_SALAS][MAX_ASIENTOS],
                       int n_asientos_salas[MAX_SALAS], int salas_clientes[MAX_SALAS][MAX_CLIENTES],
                       int n_salas_clientes[MAX_SALAS]) {
    int nro_sala = m->op.elegir_sala.nro_sala;
    for (int i = 0; i < MAX_CLIENTES; ++i) {
        // PONGO EL NRO DE CLIENTE EN LA PRIMERA POSICIÓN LIBRE
        if (salas_clientes[nro_sala][n_salas_clientes[nro_sala]] == 0) {
            salas_clientes[nro_sala][n_salas_clientes[nro_sala]] = m->mtype;
            break;
        }
    }
    printf("%sCLIENTE %i en la sala %i\n", KBLU, m->mtype, nro_sala + 1);
    n_salas_clientes[nro_sala]++; // entra un nuevo cliente a la sala;

    mostrar_asientos(nro_sala, asientos_salas);

    memcpy(m->op.info_asientos.asiento_habilitado, asientos_salas[nro_sala], MAX_ASIENTOS * sizeof(int));


    m->op.info_asientos.cant_asientos = n_asientos_salas[nro_sala];
    m->tipo = INFORMAR_ASIENTOS;
    m->op.info_asientos.nro_sala = nro_sala;
    printf("[ADMIN] Enviando INFORMAR_ASIENTOS\n");

    canal_enviar(canal_cine_admin, *m);
}

void informar_salas(mensaje_t *m, canal *canal_cine_admin, int cine_id, int n_salas, int n_asientos_salas[MAX_SALAS]) {
    memcpy(m->op.info_salas.asientos_por_sala, n_asientos_salas, MAX_SALAS * sizeof(int));
    m->op.info_salas.cant_salas = n_salas;
    printf("[ADMIN] Enviando INFORMAR_SALAS\n");
    canal_enviar(canal_cine_admin, *m);
}

void notificar_clientes(int q_admin_cliente, int nro_sala, int asientos_salas[MAX_SALAS][MAX_ASIENTOS],
                        int n_asientos_salas[MAX_SALAS],
                        int salas_clientes[MAX_SALAS][MAX_CLIENTES]) {
    // Si se pudo reservar alguno de los asientos, les aviso a los clientes que estaban mirando esa sala

    mensaje_t msg;
    msg.tipo = INFORMAR_ASIENTOS;
    msg.op.info_asientos.nro_sala = nro_sala;
    msg.op.info_asientos.cant_asientos = n_asientos_salas[nro_sala];
    memcpy(msg.op.info_asientos.asiento_habilitado, asientos_salas[nro_sala], MAX_ASIENTOS * sizeof(int));
    printf("%sNOTIFICANDO CLIENTES\n", KMAG);
    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (salas_clientes[nro_sala][i] != 0) {
            printf("%sNOTIFICANDO CLIENTE %i\n", KMAG, salas_clientes[nro_sala][i]);
            // 0 si no hay cliente en esa posicion, si no, es el pid del cliente, != 0
            msg.mtype = salas_clientes[nro_sala][i];
            printf("%sENVIO MENSAJE CON MTYPE %i EN LA COLA%i\n", KCYN, msg.mtype, q_admin_cliente);
            msg_queue_send(q_admin_cliente, &msg);
        }
    }
}


void informar_reserva(mensaje_t *m, canal *canal_cine_admin, int cine_id, int asientos_salas[MAX_SALAS][MAX_ASIENTOS],
                      int n_asientos_salas[MAX_SALAS], int *n_sala, int *n_reservados,
                      int reservas[MAX_SALAS][MAX_ASIENTOS]) {
    mensaje_t msg;
    msg.tipo = INFORMAR_RESERVA;
    int cli_id = m->mtype;
    msg.mtype = cine_id;
    memset(msg.op.info_reserva.asientos_reservados, 0, MAX_ASIENTOS_RESERVADOS * sizeof(int));

    int nro_asientos_pedidos = m->op.elegir_asientos.cant_elegidos;
    int asientos_reservados = 0;
    int nro_sala = m->op.elegir_asientos.nro_sala; // ***

    *n_sala = nro_sala; // Para poder usarlo después

    printf("[ADMIN] Veo si se pueden reservar %i asientos en la sala %i\n", nro_asientos_pedidos, nro_sala + 1);
    for (int i = 0; i < nro_asientos_pedidos; i++) {
        int nro_asiento = m->op.elegir_asientos.asientos_elegidos[i];
        if (asientos_salas[nro_sala][nro_asiento] == DISPONIBLE) {
            msg.op.info_reserva.asientos_reservados[i] = 1;
            asientos_salas[nro_sala][nro_asiento] = RESERVADO;
            reservas[nro_sala][nro_asiento] = cli_id;
            asientos_reservados++;
            printf("%s[ADMIN] Un asiento reservado: %i para el cliente %i\n", KCYN, nro_asiento, cli_id);
        } else {
            msg.op.info_reserva.asientos_reservados[i] = 0;
        }
    }

    msg.op.info_reserva.cant_reservados = asientos_reservados;
    *n_reservados = asientos_reservados; // Para usarlo después
    n_asientos_salas[nro_sala] -= asientos_reservados; // Descuento los asientos reservados de la sala
    printf("[ADMIN] Enviando INFORMAR_RESERVA\n");
    canal_enviar(canal_cine_admin, msg);
}

void informar_pago(mensaje_t *m, canal *canal_cine_admin, int cine_id) {
    mensaje_t msg;
    msg.tipo = INFORMAR_PAGO;
    msg.mtype = cine_id;
    msg.op.info_pago.precio = 500;
    printf("[ADMIN] Enviando INFORMAR_PAGO\n");
    canal_enviar(canal_cine_admin, msg);
}

void pago_ok(mensaje_t *m, canal *canal_cine_admin, int cine_id) {
    mensaje_t msg;
    msg.tipo = PAGO_OK;
    msg.mtype = cine_id;
    msg.op.pago_ok.pago_ok = 500;
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


void cancelar_reservas(int reservas[MAX_SALAS][MAX_ASIENTOS], int asientos_salas[MAX_SALAS][MAX_ASIENTOS], int cli_id,
                       int n_sala, int n_asientos_salas[MAX_SALAS]) {
    // Hay que guardar también cuál cliente reservó cuáles asientos para poder hacer esto!
    printf("%sCANCELANDO LAS RESERVAS DEL CLIENTE %i\n", KRED, cli_id);
    for (int i = 0; i < MAX_ASIENTOS; ++i) {
        if (reservas[n_sala][i] == cli_id) {
            reservas[n_sala][i] = -1; // Libre
            asientos_salas[n_sala][i] = DISPONIBLE;
            n_asientos_salas[n_sala] += 1; // Agrego al contador de asientos disponibles
        }
    }
}

int main(int argc, char *argv[]) {
	srand(time(NULL));
    int n_salas = 1 + (rand() % MAX_SALAS); // Cantidad de salas que hay en total
    printf("Cantidad de salas: %i\n", n_salas);
    int n_asientos_salas[MAX_SALAS]; // Cantidad de asientos que hay en cada sala
    int asientos_salas[MAX_SALAS][MAX_ASIENTOS]; // Asientos disponibles/ocupados en cada sala
    int salas_clientes[MAX_SALAS][MAX_CLIENTES];
    int n_salas_clientes[MAX_SALAS];
    int reservas[MAX_SALAS][MAX_ASIENTOS]; // En cada posición está el id del cliente que reservó ese asiento
    memset(reservas, 0, sizeof(int) * MAX_SALAS * MAX_ASIENTOS);
    memset(n_salas_clientes, 0, sizeof(int) * MAX_SALAS); // Lleno con ceros -> no hay clientes
    memset(salas_clientes, 0, sizeof(int) * MAX_SALAS * MAX_CLIENTES); // Lleno con ceros -> no hay clientes

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
        std::cerr << "[ADMIN]Error al crear canal de comunicacion entre admin y cine " << std::endl;
        exit(1);
    }


    // Entre admin y clientes es solo en una dirección por eso uso sólo una cola y no "canal"
    int q_admin_cliente = msg_queue_get(Q_ADMIN_CLI);
    if (q_admin_cliente == -1) {
        std::cerr << "[ADMIN]Error al crear canal de comunicacion entre admin y cliente" << std::endl;
        exit(1);
    }

    while (true) {
        mensaje_t msg;
        printf("%s[ADMIN] Esperando mensaje\n", KGRN);
        long cine_id = canal_recibir(canal_cine_admin, msg, 0); // Agarro cualquier mensaje que me hayan mandado
        printf("%s[ADMIN] Mensaje recibido\n", KGRN);
        printf("%s[ADMIN] Recibí mensaje con tipo %i y mtype %i\n", KGRN, msg.tipo, msg.mtype);
        if (msg.tipo == INFORMAR_SALAS) {
            printf("%s[ADMIN] INFORMAR_SALAS para %i\n", KGRN, cine_id);
            informar_salas(&msg, canal_cine_admin, cine_id, n_salas, n_asientos_salas);

        }

        if (msg.tipo == ELEGIR_SALA) {
            printf("%s[ADMIN] INFORMAR_ASIENTOS para %i\n", KGRN, cine_id);
            printf("NRO DE SALA RECIBIDO ADMIN: %i\n", msg.op.elegir_sala.nro_sala + 1);
            informar_asientos(&msg, canal_cine_admin, cine_id, asientos_salas, n_asientos_salas, salas_clientes,
                              n_salas_clientes);

        }

        if (msg.tipo == ELEGIR_ASIENTOS) {
            printf("%s[ADMIN] INFORMAR_RESERVA para %i\n", KGRN, cine_id);
            int n_sala;
            int n_reservados;
            informar_reserva(&msg, canal_cine_admin, cine_id, asientos_salas, n_asientos_salas, &n_sala, &n_reservados,
                             reservas);
            printf("Se reservaron %i asientos para el cliente %i\n", n_reservados, msg.mtype);
            if (n_reservados > 0) {
                notificar_clientes(q_admin_cliente, n_sala, asientos_salas, n_asientos_salas, salas_clientes);
            }

        }

        if (msg.tipo == CONFIRMAR_RESERVA) {
            printf("%s[ADMIN] INFORMAR_PAGO para %i\n", KGRN, cine_id);
            informar_pago(&msg, canal_cine_admin, cine_id);

        }

        if (msg.tipo == PAGAR) {
            int cli_id = msg.mtype;
            pago_ok(&msg, canal_cine_admin, cine_id);
            // SE PUEDE REEMPLAZAR POR UN MAP, PERO YA QUE ESTAMOS HACIENDO CASI TODO EN C, LO DEJO ASÍ
            // UN MAP DE SALAS -> CLIENTES
            // Y OTRO DE CLIENTES -> SALAS
            for (int i = 0; i < MAX_SALAS; ++i) {
                for (int j = 0; j < MAX_CLIENTES; ++j) {
                    // SACO AL CLIENTE DEL LISTADO DE CLIENTES DE LA SALA
                    if (salas_clientes[i][j] == cli_id) {
                        printf("%sEL CLIENTE %i YA PAGÓ, LOGOUT%s\n", KRED, cli_id, KNRM);
                        salas_clientes[i][j] = 0;
                        break;
                    }
                }
            }

        }

        if (msg.tipo == TIMEOUT) {
            // Cancelo la reserva que había hecho el cliente, si es que había alguna y le aviso a los demás clientes

            int cli_id = msg.op.timeout.cli_id;
            int n_sala = msg.op.timeout.n_sala; // NRO DE SALA EN AL QUE ESTABA EL CLIENTE
            cancelar_reservas(reservas, asientos_salas, cli_id, n_sala, n_asientos_salas);
            notificar_clientes(q_admin_cliente, n_sala, asientos_salas, n_asientos_salas, salas_clientes);

        }

    }

}
