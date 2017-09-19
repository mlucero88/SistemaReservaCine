#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <sys/wait.h>

#include "../common/canal.h"
#include "../common/ipc/sh_mem.h"
#include "../common/color_print.h"

#define CLI_LOG cli_log
#define CLI_PRINTF(fmt, ...) FPRINTF(CLI_LOG, BLUE, fmt, ##__VA_ARGS__)

int shmemId;
pid_t pid_asyn = 0;
canal *canal_cli_cine = nullptr;
shmem *sharedMem = nullptr;
FILE *cli_log = nullptr;

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

void login() {
    mensaje_t msg = {.mtype = LOGIN_MSG_TYPE, .tipo = LOGIN};
    msg.op.login.cli_id = getpid();
    if (!canal_enviar(canal_cli_cine, msg)) {
        CLI_PRINTF("Error al enviar mensaje de LOGIN: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("LOGIN enviado");
}

mensaje_t recibir_info_salas() {
    mensaje_t msg;
    if (!canal_recibir(canal_cli_cine, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de INFORMAR_SALAS: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != INFORMAR_SALAS) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
        } else {
            CLI_PRINTF("Error: se esperaba INFORMAR_SALAS y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("INFORMAR_SALAS recibido");

    return msg;
}

void enviar_elegir_sala(int sala) {
    mensaje_t msg = {.mtype = getpid(), .tipo = ELEGIR_SALA};
    msg.op.elegir_sala.nro_sala = sala;
    if (!canal_enviar(canal_cli_cine, msg)) {
        CLI_PRINTF("Error al enviar mensaje de ELEGIR_SALA: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("ELEGIR_SALA enviado");
}

mensaje_t recibir_info_sala() {
    mensaje_t msg;
    if (!canal_recibir(canal_cli_cine, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de INFORMAR_ASIENTOS: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != INFORMAR_ASIENTOS) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
        } else {
            CLI_PRINTF("Error: se esperaba INFORMAR_ASIENTOS y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("INFORMAR_ASIENTOS recibido");

    return msg;
}

void elegir_asientos(int asientos[], int cantAsientos, int sala) {
    mensaje_t msg = {.mtype = getpid(), .tipo = ELEGIR_ASIENTOS};
    msg.op.elegir_asientos.nro_sala = sala;
    memcpy(msg.op.elegir_asientos.asientos_elegidos, asientos, sizeof(int) * cantAsientos);
    msg.op.elegir_asientos.cant_elegidos = cantAsientos;
    if (!canal_enviar(canal_cli_cine, msg)) {
        CLI_PRINTF("Error al enviar mensaje de ELEGIR_ASIENTOS: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("ELEGIR_ASIENTOS enviado");
}

mensaje_t recibir_info_reserva() {
    mensaje_t msg;
    if (!canal_recibir(canal_cli_cine, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de INFORMAR_RESERVA: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != INFORMAR_RESERVA) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
            printf("Tardaste mucho :(\n");
        } else {
            CLI_PRINTF("Error: se esperaba INFORMAR_RESERVA y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("INFORMAR_RESERVA recibido");

    return msg;
}

void confirmar_reserva(bool aceptar) {
    mensaje_t msg = {.mtype = getpid(), .tipo = CONFIRMAR_RESERVA};
    msg.op.confirmar_reserva.reserva_confirmada = aceptar ? 1 : 0;
    if (!canal_enviar(canal_cli_cine, msg)) {
        CLI_PRINTF("Error al enviar mensaje de CONFIRMAR_RESERVA: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("CONFIRMAR_RESERVA enviado");
}

mensaje_t recibir_info_pago() {
    mensaje_t msg;
    if (!canal_recibir(canal_cli_cine, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de INFORMAR_PAGO: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != INFORMAR_PAGO) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
        } else {
            CLI_PRINTF("Error: se esperaba INFORMAR_PAGO y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("INFORMAR_PAGO recibido");

    return msg;
}

void pagar(int precio) {
    mensaje_t msg = {.mtype = getpid(), .tipo = PAGAR};
    msg.op.pagar.pago = precio;
    if (!canal_enviar(canal_cli_cine, msg)) {
        CLI_PRINTF("Error al enviar mensaje de PAGAR: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("PAGAR enviado");
}

void recibir_pago_ok() {
    mensaje_t msg;
    if (!canal_recibir(canal_cli_cine, msg, getpid())) {
        CLI_PRINTF("Error al recibir mensaje de PAGO_OK: %s", strerror(errno));
        liberarYSalir();
    }
    if (msg.tipo != PAGO_OK) {
        if (msg.tipo == TIMEOUT) {
            CLI_PRINTF("TIMEOUT recibido");
        } else {
            CLI_PRINTF("Error: se esperaba PAGO_OK y se recibio %s", strOpType(msg.tipo));
        }
        liberarYSalir();
    }
    CLI_PRINTF("PAGO_OK recibido");
}

int main() {
    int cli_id = getpid();
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

    entidad_t cliente = {.proceso = entidad_t::CLIENTE, .pid = cli_id};
    entidad_t cine = {.proceso = entidad_t::CINE, .pid = -1};
    mensaje_t msg;

    canal_cli_cine = canal_crear(cliente, cine);
    if (canal_cli_cine == NULL) {
        CLI_PRINTF("Error al crear canal de comunicacion entre cliente y cine: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("Canal de comunicacion entre cliente y cine creado");

    sharedMem = sh_mem_create(cli_id);
    if (sharedMem == NULL) {
        CLI_PRINTF("Error al crear memoria compartida entre cliente y cliente_asyn: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("Memoria compartida entre cliente y cliente_asyn creada");

	if ((pid_asyn = fork()) == 0) {
		char cli_id_str[16];
		sprintf(cli_id_str, "%d", cli_id);
        execl("./cliente_asyn", "cliente_asyn", cli_id_str, NULL);
		CLI_PRINTF("Error en el execl del cliente asincronico");
		exit(1);
	}
	CLI_PRINTF("Cliente asincronico iniciado (pid=%d)", pid_asyn);


    /******** LOGIN ********/

    login();

    /***** RECIBIR INFO SALAS *****/

    msg = recibir_info_salas();
    printf("\nCantidad de salas: %i\n", msg.op.info_salas.cant_salas);
    for (int i = 0; i < msg.op.info_salas.cant_salas; i++) {
        printf("SALA %i -> %i\n", i + 1, msg.op.info_salas.asientos_por_sala[i]);
    }

    /******* ELEGIR SALA ********/

    int sala;
    bool salaElegida = false;
    do {
        printf("\nElija sala: ");
        char sala_str[12];
        fgets(sala_str, 12, stdin);
        if(strcmp(sala_str, "\n") == 0) {
        	continue;
        }
        strtok(sala_str, "\n");
        sala = atoi(sala_str) - 1;
        if (sala < 0 || sala >= msg.op.info_salas.cant_salas) {
            printf("Nro de sala invalida!\n");
        } else if (msg.op.info_salas.asientos_por_sala[sala] <= 0) {
            printf("Sala llena!\n");
        } else {
            salaElegida = true;
        }
    } while (!salaElegida);

    enviar_elegir_sala(sala);

    /******* RECIBIR INFO SALA *********/

    msg = recibir_info_sala();
    printf("\nHay %i asientos en total en la sala\n", msg.op.info_asientos.cant_asientos);
    for (int j = 0; j < MAX_ASIENTOS; j++) {
        printf("%c", msg.op.info_asientos.asiento_habilitado[j] == DISPONIBLE ? 'O' : 'X');
    }
    printf("\n");

    /******* ELEGIR ASIENTOS *********/

    int asiento_habilitado[MAX_ASIENTOS], asientosEnSala = msg.op.info_asientos.cant_asientos;
    memcpy(asiento_habilitado, msg.op.info_asientos.asiento_habilitado, MAX_ASIENTOS * sizeof(int));

    printf("ENTER para elegir asientos (a partir de ahora no se actualizaran los asientos de la sala)");
    char dummy[12];
    fgets(dummy, 12, stdin);

    shmem_data sharedData;
    sh_mem_read(sharedMem, &sharedData);
    if (sharedData.dirty) {
        memcpy(asiento_habilitado, sharedData.asientos, MAX_ASIENTOS * sizeof(int));
        asientosEnSala = sharedData.cantidad;
        printf("\nATENCION! Ahora hay %i asientos en total en la sala\n", asientosEnSala);
        for (int j = 0; j < asientosEnSala; j++) {
            printf("%c", asiento_habilitado[j] == DISPONIBLE ? 'O' : 'X');
        }
        printf("\n");
    }

    int asientos_elegidos[MAX_ASIENTOS_RESERVADOS], asiento, cantAsientos = 0;
    memset(asientos_elegidos, 0, MAX_ASIENTOS_RESERVADOS * sizeof(int));

    bool finEleccionAsientos = false;
    do {
        printf("\nElija un asiento (-1 para finalizar eleccion)(el primer asiento es 0): ");
        char asiento_str[12];
        fgets(asiento_str, 12, stdin);
        if(strcmp(asiento_str, "\n") == 0) {
        	continue;
        }
        strtok(asiento_str, "\n");
        asiento = atoi(asiento_str);
        if (asiento == -1) {
            if (cantAsientos > 0) {
                finEleccionAsientos = true;
            } else {
                printf("Debe elegir al menos un asiento!\n");
            }
        } else if (asiento >= 0 && asiento < asientosEnSala) {
        	if(asiento_habilitado[asiento] == DISPONIBLE) {
        		asientos_elegidos[cantAsientos++] = asiento;
        		asiento_habilitado[asiento] = RESERVADO;
        		printf("Asiento %i agregado\n", asiento);
        		if (cantAsientos == MAX_ASIENTOS_RESERVADOS) {
        			finEleccionAsientos = true;
        		}
        	}
        	else {
        		printf("Asiento no disponible\n");
        	}
        } else {
            printf("Nro de asiento invalido!\n");
        }
    } while (!finEleccionAsientos);

    elegir_asientos(asientos_elegidos, cantAsientos, sala);

    /******* RECIBIR INFO RESERVA *********/

    msg = recibir_info_reserva();
    printf("\nSe reservaron %i/%i asientos: ", msg.op.info_reserva.cant_reservados, cantAsientos);
    for (int i = 0; i < MAX_ASIENTOS_RESERVADOS; i++) {
        if (msg.op.info_reserva.asientos_reservados[i]) {
            printf(" %i", asientos_elegidos[i]);
        }
    }
    printf("\n");

    /******* CONFIRMAR RESERVA *********/

    bool aceptarReserva = false;
    while(1) {
    	fflush(stdin);
        printf("\nDesea confirmar la reserva [S/N]? ");
        char c[12];
        fgets(c, 12, stdin); // Por si el cliente escribe cosas de mas
        if(c[0] == 'S' || c[0] == 's') {
        	aceptarReserva = true;
        	break;
        }
        else if(c[0] == 'N' || c[0] == 'n') {
        	break;
        }
        printf("Respuesta invalida!\n");
    }

    confirmar_reserva(aceptarReserva);

    if(!aceptarReserva) {
    	printf("\nReserva cancelada. Gracias por operar con red SARASA\n");
    	liberarYSalir();
    }

    /******* RECIBIR INFO PAGO *********/

    msg = recibir_info_pago();
    printf("\nSu total a abonar es: $%d\n", msg.op.info_pago.precio);

    /******* CONFIRMAR RESERVA *********/
    printf("\nPresione enter para enviar el pago...");
    getchar();
    pagar(msg.op.info_pago.precio);

    /******* RECIBIR PAGO ACEPTADO *********/
    printf("Esperando respuesta de pago...\n");
    recibir_pago_ok();
    printf("\nPago aceptado. Gracias por operar con red SARASA\n");

    liberarYSalir();

    return 0;
}
