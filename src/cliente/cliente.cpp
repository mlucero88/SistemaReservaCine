#include <cstring>
#include <cstdlib>
#include <csignal>

#include "interfaz.h"
#include "../common/color_print.h"

#define CLI_LOG(fmt, ...) FPRINTF(stdout, KCYN, fmt, ##__VA_ARGS__)

bool pedir_confirmacion_reserva();

void mostrar_info_salas(op_info_salas_t info_salas);

int pedir_asientos(op_info_asientos_t info_asientos, int asientos_elegidos[MAX_ASIENTOS_RESERVADOS]);

int pedir_sala(op_info_salas_t info_salas);

void mostrar_asientos_reservados(int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos,
                                 op_info_reserva_t info_reserva);

void mostrar_asientos(op_info_asientos_t info_asientos);

void liberar_y_salir() {
	exit(1);
}

void sighandler(int signum) {
    if (signum == SIGINT) {
    	liberar_y_salir();
    }
}

int main(int argc, char* argv[]) {
    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);
    signal(SIGINT, sighandler);

    CLI_LOG("PID: %i\n\n", getpid());

    /* INICIALIZAR */
    m_id mom_id = m_init();
    if(m_errno != RET_OK) {
    	CLI_LOG("Error en la inicializacion: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }
//    CLI_LOG("MOM incializado ok\n");

    /* LOGIN */
    op_info_salas_t info_salas = m_login(mom_id);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error en el login: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }
//    CLI_LOG("LOGIN ok\n");

    /* ELEGIR UNA SALA */
    mostrar_info_salas(info_salas);
    int nro_sala = pedir_sala(info_salas);
    op_info_asientos_t info_asientos = m_seleccionar_sala(mom_id, nro_sala);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error al seleccionar sala: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }

    /* ELEGIR ASIENTOS DENTRO DE LA SALA ELEGIDA */
    mostrar_asientos(info_asientos);
    int asientos_elegidos[MAX_ASIENTOS_RESERVADOS];
    int cant_elegidos = pedir_asientos(info_asientos, asientos_elegidos);
    CLI_LOG("Se pidieron %i asientos\n", cant_elegidos);
    op_info_reserva_t info_reserva = m_seleccionar_asientos(mom_id, asientos_elegidos, cant_elegidos);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error al reservar los asientos: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }

    /* CONFIRMAR LA RESERVA */
    mostrar_asientos_reservados(asientos_elegidos, cant_elegidos, info_reserva);
    bool confirmacion = pedir_confirmacion_reserva();
    op_info_pago_t info_pago = m_confirmar_reserva(mom_id, confirmacion);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error al confirmar la reserva: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }
    if (!confirmacion) {
        printf("\nReserva cancelada. Gracias por operar con red SARASA\n");
        liberar_y_salir();
    }

    /* PAGAR */
    int precio = info_pago.precio;
    printf("\nSu total a abonar es: $%d\n", precio);
    printf("\nPresione enter para enviar el pago...");
    getchar();
    printf("Esperando respuesta de pago...\n");
    m_pagar(mom_id, precio);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error al pagar la reserva: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }

    printf("\nPago aceptado. Gracias por operar con red SARASA\n");
    liberar_y_salir();
}

void mostrar_asientos_reservados(int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos,
                                 op_info_reserva_t info_reserva) {
    int *asientos_reservados = info_reserva.asientos_reservados;
    int cant_reservados = info_reserva.cant_reservados;
    printf("\nSe reservaron %i/%i asientos: ", cant_reservados, cant_elegidos);
    for (int i = 0; i < MAX_ASIENTOS_RESERVADOS; i++) {
        if (asientos_reservados[i]) {
            printf(" %i", asientos_elegidos[i]);
        }
    }
    printf("\n");
}

int pedir_asientos(op_info_asientos_t info_asientos,
                   int asientos_elegidos[MAX_ASIENTOS_RESERVADOS]) {
    int *asientos_habilitados = info_asientos.asiento_habilitado;
    int cant_asientos = info_asientos.cant_asientos;
    int asiento, cant_elegidos = 0;
    memset(asientos_elegidos, 0, MAX_ASIENTOS_RESERVADOS * sizeof(int));

    bool finEleccionAsientos = false;
    do {
        printf("\nElija un asiento (-1 para finalizar eleccion)(el primer asiento es 0): ");
        char asiento_str[12];
        fgets(asiento_str, 12, stdin);
        if (strcmp(asiento_str, "\n") == 0) {
            continue;
        }
        strtok(asiento_str, "\n");
        asiento = atoi(asiento_str);
        if (asiento == -1) {
            if (cant_elegidos > 0) {
                finEleccionAsientos = true;
            } else {
                printf("Debe elegir al menos un asiento!\n");
            }
        } else if (asiento >= 0 && asiento < MAX_ASIENTOS) {
            if (asientos_habilitados[asiento] == DISPONIBLE) {
                asientos_elegidos[cant_elegidos++] = asiento;
                asientos_habilitados[asiento] = RESERVADO;
                printf("Asiento %i agregado\n", asiento);
                if (cant_elegidos == MAX_ASIENTOS_RESERVADOS) {
                    finEleccionAsientos = true;
                }
            } else {
                printf("Asiento no disponible\n");
            }
        } else {
            printf("Nro de asiento invalido!\n");
        }
    } while (!finEleccionAsientos);
    return cant_elegidos;
}

bool pedir_confirmacion_reserva() {
    bool aceptarReserva = false;
    while (true) {
        fflush(stdin);
        printf("\nDesea confirmar la reserva [S/N]? ");
        char c[12];
        fgets(c, 12, stdin); // Por si el cliente escribe cosas de mas
        if (c[0] == 'S' || c[0] == 's') {
            aceptarReserva = true;
            break;
        } else if (c[0] == 'N' || c[0] == 'n') {
            break;
        }
        printf("Respuesta invalida!\n");
    }
    return aceptarReserva;
}

void mostrar_info_salas(op_info_salas_t info_salas) {
    int *asientos_por_sala = info_salas.asientos_por_sala;
    int cant_salas = info_salas.cant_salas;
    printf("\nCantidad de salas: %i\n", cant_salas);
    for (int i = 0; i < info_salas.cant_salas; i++) {
        printf("SALA %i -> %i\n", i + 1, asientos_por_sala[i]);
    }
}


int pedir_sala(op_info_salas_t info_salas) {
    int *asientos_por_sala = info_salas.asientos_por_sala;
    int cant_salas = info_salas.cant_salas;
    int nro_sala;
    bool salaElegida = false;
    do {
        printf("\nElija sala: ");
        char sala_str[12];
        fgets(sala_str, 12, stdin);
        if (strcmp(sala_str, "\n") == 0) {
            continue;
        }
        strtok(sala_str, "\n");
        nro_sala = atoi(sala_str) - 1;
        if (nro_sala < 0 || nro_sala >= cant_salas) {
            printf("Nro de sala invalida!\n");
        } else if (asientos_por_sala[nro_sala] <= 0) {
            printf("Sala llena!\n");
        } else {
            salaElegida = true;
        }
    } while (!salaElegida);
    return nro_sala;
}

void mostrar_asientos(op_info_asientos_t info_asientos) {
    for (int j = 0; j < MAX_ASIENTOS; j++) {
        printf("%03i ", j);
    }
    printf("\n");
    for (int j = 0; j < MAX_ASIENTOS; j++) {
        printf(" %c  ", info_asientos.asiento_habilitado[j] == DISPONIBLE ? 'O' : 'X');
    }
    printf("\n");
}
