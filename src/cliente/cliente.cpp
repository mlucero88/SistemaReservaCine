
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <sys/wait.h>

#include "../common/canal.h"
#include "../common/ipc/sh_mem.h"
#include "../common/color_print.h"
#include "interfaz.h"

#define CLI_LOG cli_log
#define CLI_PRINTF(fmt, ...) FPRINTF(CLI_LOG, BLUE, fmt, ##__VA_ARGS__)

int shmemId;
pid_t pid_asyn = 0;
canal *canal_cli_cine = nullptr;
shmem *sharedMem = nullptr;
FILE *cli_log = nullptr;

bool pedir_confirmacion_reserva();

void mostrar_info_salas(int asientos_por_sala[MAX_SALAS], int cant_salas);

int pedir_asientos(int asientos_habilitados[MAX_ASIENTOS], int cant_asientos,
                   int asientos_elegidos[MAX_ASIENTOS_RESERVADOS]);

int pedir_sala(int asientos_por_sala[MAX_SALAS], int cant_salas);

void mostrar_asientos_reservados(int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos,
                                 int asientos_reservados[MAX_ASIENTOS_RESERVADOS], int cant_reservados);

void mostrar_asientos(int asientos[MAX_ASIENTOS]) {
    for (int j = 0; j < MAX_ASIENTOS; j++) {
        printf("%c", asientos[j] == DISPONIBLE ? 'O' : 'X');
    }
    printf("\n");
}

void liberarYSalir() {
    CLI_PRINTF("Preparando para salir...");
    if (pid_asyn > 0) {
        int status;
        kill(pid_asyn, SIGINT);
        wait(&status);
        CLI_PRINTF("Cliente asincronico cerrado");
    }

    if (sharedMem != nullptr) {
        sh_mem_destroy(sharedMem);
        CLI_PRINTF("Memoria compartida liberada");
    }
    if (canal_cli_cine != nullptr) {
        canal_destruir(canal_cli_cine);
        CLI_PRINTF("Canal destruido");
    }

    CLI_PRINTF("*** Aplicacion cerrada ***");
    fclose(cli_log);
    exit(1);
}

void sighandler(int signum) {
    if (signum == SIGINT) {
        liberarYSalir();
    }
}

int main() {
    int cli_id = getpid();
    printf("PID: %i\n", cli_id);
    char logName[64];
    sprintf(logName, "../logs/cli_%i" ".log", cli_id);
    cli_log = fopen(logName, "w");
    signal(SIGINT, sighandler);

    struct sigaction sigchld;
    sigchld.sa_handler = SIG_DFL;
    sigchld.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sigchld.sa_mask);
    sigaction(SIGCHLD, &sigchld, NULL);

    CLI_PRINTF("*** Aplicacion iniciada ***");

    /* INICIALIZAR */

    m_id mom_id = m_init();
    int result;

    /* LOGIN */

    result = m_login(mom_id, cli_id);
    if (result != 0) {
        CLI_PRINTF("ERROR: no se pudo hacer LOGIN");
    }

    /* INFO GENERAL DE LAS SALAS */
    int info_salas[MAX_SALAS];
    int cant_salas;
    result = m_info_salas(mom_id, info_salas, &cant_salas);
    if (result != 0) {
        CLI_PRINTF("ERROR: no se pudo obtener la info de las salas");
    }
    mostrar_info_salas(info_salas, cant_salas);

    /* ELEGIR UNA SALA */
    int asientos_habilitados[MAX_ASIENTOS];
    int cant_asientos;
    int nro_sala = pedir_sala(info_salas, cant_salas);
    result = m_asientos_sala(mom_id, nro_sala, asientos_habilitados, &cant_asientos);
    if (result != 0) {
        CLI_PRINTF("ERROR: no se pudo obtener la info de las sala elegida");
    }
    mostrar_asientos(asientos_habilitados);
    /* ELEGIR ASIENTOS DENTRO DE LA SALA ELEGIDA */

    int asientos_elegidos[MAX_ASIENTOS_RESERVADOS];
    int cant_elegidos = pedir_asientos(asientos_habilitados, cant_asientos, asientos_elegidos);
    int asientos_reservados[MAX_ASIENTOS_RESERVADOS];
    int cant_reservados;
    result = m_reservar_asientos(mom_id, asientos_elegidos, cant_elegidos, nro_sala, asientos_reservados,
                                 &cant_reservados);
    if (result != 0) {
        CLI_PRINTF("ERROR: no se pudo reservar los asientos");
    }
    printf("\nSe reservaron %i/%i asientos: ", cant_reservados, cant_elegidos);
    mostrar_asientos_reservados(asientos_elegidos, cant_elegidos, asientos_reservados, cant_reservados);

    /* CONFIRMAR LA RESERVA */

    bool confirmacion = pedir_confirmacion_reserva();
    if (!confirmacion) {
        printf("\nReserva cancelada. Gracias por operar con red SARASA\n");
        liberarYSalir();
    }
    int precio;
    result = m_confirmar_reserva(mom_id, confirmacion, &precio);
    if (result != 0) {
        CLI_PRINTF("ERROR: no se pudo confirmar la reserva");
    }

    /* PAGAR */

    printf("\nSu total a abonar es: $%d\n", precio);

    printf("\nPresione enter para enviar el pago...");

    getchar();

    printf("Esperando respuesta de pago...\n");

    result = m_pagar(mom_id);
    if (result != 0) {
        CLI_PRINTF("ERROR: no se pudo confirmar la reserva");
    }
    printf("\nPago aceptado. Gracias por operar con red SARASA\n");
    liberarYSalir();
}

void mostrar_asientos_reservados(int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], int cant_elegidos,
                                 int asientos_reservados[MAX_ASIENTOS_RESERVADOS], int cant_reservados) {
    printf("\nSe reservaron %i/%i asientos: ", cant_reservados, cant_elegidos);
    for (int i = 0; i < MAX_ASIENTOS_RESERVADOS; i++) {
        if (asientos_reservados[i]) {
            printf(" %i", asientos_elegidos[i]);
        }
    }
    printf("\n");
}

int pedir_asientos(int asientos_habilitados[MAX_ASIENTOS], int cant_asientos,
                   int asientos_elegidos[MAX_ASIENTOS_RESERVADOS]) {
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
        } else if (asiento >= 0 && asiento < cant_asientos) {
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

void mostrar_info_salas(int asientos_por_sala[MAX_SALAS], int cant_salas) {
    printf("\nCantidad de salas: %i\n", cant_salas);
    for (int i = 0; i < cant_salas; i++) {
        printf("SALA %i -> %i\n", i + 1, asientos_por_sala[i]);
    }
}


int pedir_sala(int asientos_por_sala[MAX_SALAS], int cant_salas) {

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