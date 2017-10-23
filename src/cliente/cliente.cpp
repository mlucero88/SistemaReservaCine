#include <cstring>
#include <cstdlib>
#include <csignal>

#include "interfaz.h"
#include "../common/color_print.h"

#define CLI_LOG(fmt, ...) FPRINTF(stdout, KCYN, fmt, ##__VA_ARGS__)

static uuid_t cli_id;

int pedir_sala(const op_info_salas_t& info_salas);

int pedir_asientos(op_info_asientos_t& info_asientos, int asientos_elegidos[MAX_ASIENTOS_RESERVADOS]);

bool pedir_confirmacion_reserva();

void mostrar_info_salas(const op_info_salas_t& info_salas);

void mostrar_asientos(const op_info_asientos_t& info_asientos);

void mostrar_asientos_reservados(const int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos,
                                 const op_info_reserva_t& info_reserva);

void liberar_y_salir() {
	m_dest(cli_id);
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

    op_info_asientos_t info_asientos;
    info_asientos.nro_sala = -1;
    int asientos_elegidos[MAX_ASIENTOS_RESERVADOS];
    memset(asientos_elegidos, -1, MAX_ASIENTOS_RESERVADOS * sizeof(int));

    CLI_LOG("PID: %i\n\n", getpid());

    /* INICIALIZAR */
    cli_id = m_init();
    if(m_errno != RET_OK) {
    	CLI_LOG("Error en la inicializacion: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }

    /* REGISTRO DE CALLBACK */
    m_reg_cb_actualizacion_sala(cli_id, [&info_asientos, &asientos_elegidos](const op_info_asientos_t& nueva_info) {
    	/* Lo correcto seria lockear esta porcion de codigo junto con pedir_asientos, pero como es una vista
    	 * sencilla que escapa de la idea del tp, omitimos lockear */
    	/* Actualizamos el vector que usamos para leer las reservas del usuario */
    	if(info_asientos.nro_sala == nueva_info.nro_sala) {
    		for (int nro_asiento = 0; nro_asiento < MAX_ASIENTOS; nro_asiento++) {
    			/* Veo si el estado del asiento cambio */
    			if(info_asientos.asiento_habilitado[nro_asiento] != nueva_info.asiento_habilitado[nro_asiento]) {
    				/* Me aseguro q el cambio no haya sido por pedir_asientos() */
    				bool cambiar = true;
    				for(int k = 0; k < MAX_ASIENTOS_RESERVADOS; ++k) {
    					if(nro_asiento == asientos_elegidos[k]) {
    						/* Fue cambio por pedir_asiento() */
    						cambiar = false;
    						break;
    					}
    				}
    				if(cambiar) {
    					info_asientos.asiento_habilitado[nro_asiento] = nueva_info.asiento_habilitado[nro_asiento];
    				}
    			}
    		}
    		mostrar_asientos(info_asientos);
    	}
    });

    /* LOGIN */
    op_info_salas_t info_salas = m_login(cli_id);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error en el login: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }

    /* ELEGIR UNA SALA */
    mostrar_info_salas(info_salas);
    int nro_sala = pedir_sala(info_salas);
    info_asientos = m_seleccionar_sala(cli_id, nro_sala);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error al seleccionar sala: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }

    /* ELEGIR ASIENTOS DENTRO DE LA SALA ELEGIDA */
    int cant_elegidos = pedir_asientos(info_asientos, asientos_elegidos);
    CLI_LOG("Se pidieron %i asientos\n", cant_elegidos);
    op_info_reserva_t info_reserva = m_seleccionar_asientos(cli_id, asientos_elegidos, cant_elegidos);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error al reservar los asientos: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }

    /* CONFIRMAR LA RESERVA */
    mostrar_asientos_reservados(asientos_elegidos, cant_elegidos, info_reserva);
    bool confirmacion = pedir_confirmacion_reserva();
    op_info_pago_t info_pago = m_confirmar_reserva(cli_id, confirmacion);
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
    m_pagar(cli_id, precio);
    if(m_errno != RET_OK) {
    	CLI_LOG("Error al pagar la reserva: %s\n", m_str_error(m_errno));
    	liberar_y_salir();
    }

    printf("\nPago aceptado. Gracias por operar con red SARASA\n");
    liberar_y_salir();
}

int pedir_sala(const op_info_salas_t &info_salas) {
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
        if (nro_sala < 0 || nro_sala >= info_salas.cant_salas) {
            printf("Nro de sala invalida!\n");
        } else if (info_salas.asientos_por_sala[nro_sala] <= 0) {
            printf("Sala llena!\n");
        } else {
            salaElegida = true;
        }
    } while (!salaElegida);
    return nro_sala;
}

int pedir_asientos(op_info_asientos_t& info_asientos,
                   int asientos_elegidos[MAX_ASIENTOS_RESERVADOS]) {
    int asiento, cant_elegidos = 0;

    bool finEleccionAsientos = false;
    do {
    	mostrar_asientos(info_asientos);

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
            if (info_asientos.asiento_habilitado[asiento] == DISPONIBLE) {
                asientos_elegidos[cant_elegidos++] = asiento;
                info_asientos.asiento_habilitado[asiento] = RESERVADO;
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

void mostrar_info_salas(const op_info_salas_t& info_salas) {
    printf("\nCantidad de salas: %i\n", info_salas.cant_salas);
    for (int i = 0; i < info_salas.cant_salas; i++) {
        printf("SALA %i -> %i\n", i + 1, info_salas.asientos_por_sala[i]);
    }
}

void mostrar_asientos(const op_info_asientos_t& info_asientos) {
    printf("\n");
    for (int j = 0; j < MAX_ASIENTOS; j++) {
        printf("%03i ", j);
    }
    printf("\n");
    for (int j = 0; j < MAX_ASIENTOS; j++) {
        printf(" %c  ", info_asientos.asiento_habilitado[j] == DISPONIBLE ? 'O' : 'X');
    }
    printf("\n");
}

void mostrar_asientos_reservados(const int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos,
                                 const op_info_reserva_t& info_reserva) {
    printf("\nSe reservaron %i/%i asientos: ", info_reserva.cant_reservados, cant_elegidos);
    for (int i = 0; i < MAX_ASIENTOS_RESERVADOS; i++) {
        if (info_reserva.asientos_reservados[i]) {
            printf(" %i", asientos_elegidos[i]);
        }
    }
    printf("\n");
}
