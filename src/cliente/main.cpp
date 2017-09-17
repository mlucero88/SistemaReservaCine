#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <signal.h>

#include "../common/canal.h"
#include "../common/ipc/sh_mem.h"
#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"
#include "../common/operaciones.h"

#define CLI_LOG cli_log
#define CLI_PRINTF(fmt, ...) FPRINTF(CLI_LOG, BLUE, fmt, ##__VA_ARGS__)

#ifdef _DEBUG
#define CLI_DEBUG_PRINT(fmt, ...) FPRINTF(BLUE, fmt, ##__VA_ARGS__)
#else
#define CLI_DEBUG_PRINT(fmt, ...)
#endif

struct shmem_data {
    int asientos[MAX_ASIENTOS];
    int cantidad;
};

int shmemId;
pid_t pid_asyn = 0;
canal *canal_cli_cine = nullptr;
shmem_data *sharedData = nullptr;
FILE *cli_log;

void liberarYSalir() {
    if (sharedData != nullptr) {
        sh_mem_release(static_cast<void *>(sharedData));
        sh_mem_destroy(shmemId);
    }
    if (canal_cli_cine != nullptr) {
        canal_destruir(canal_cli_cine);
    }

    if (pid_asyn > 0) {
        kill(pid_asyn, SIGKILL); // TODO hacer cierre de cli async (martin)
    }
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
    sprintf(logName, "./cli_%i" ".log", cli_id);
    cli_log = fopen(logName, "w");
    signal(SIGINT, sighandler);

    entidad_t cliente = {.proceso = entidad_t::CLIENTE, .pid = cli_id};
    entidad_t cine = {.proceso = entidad_t::CINE, .pid = -1};
    mensaje_t msg;

    canal_cli_cine = canal_crear(cliente, cine);
    if (canal_cli_cine == NULL) {
        CLI_PRINTF("Error al crear canal de comunicacion entre cliente y cine: %s", strerror(errno));
        return 1;
    }
    CLI_PRINTF("Canal de comunicacion entre cliente y cine creado");

    shmemId = sh_mem_create(cli_id, sizeof(shmem_data), reinterpret_cast<void **>(&sharedData));
    if (shmemId == -1) {
        CLI_PRINTF("Error al crear memoria compartida entre cliente y cliente_asyn: %s", strerror(errno));
        canal_destruir(canal_cli_cine);
        return 1;
    }
    CLI_PRINTF("Memoria compartida entre cliente y cliente_asyn creada");

    // TODO hacer fork (martin)
//	if ((pid_asyn = fork()) == 0) {
//		// Lanzo cliente asyn
//		char cli_id_str[12];
//		sprintf(cli_id_str, "%d", cli_id);
//		execl("./client_asyn", "client_asyn", cli_id_str, NULL);
//		exit(1);
//	}
//	CLI_PRINTF("Error al crear memoria compartida entre cliente y cliente_asyn: %s", strerror(errno));
//	printf("%sCLIENTE ASYN PARA CLIENTE %i = %i%s\n", KRED, getpid(), pid_asyn, KNRM);

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
        scanf("%d", &sala);
        if (sala < 1 || sala >= msg.op.info_salas.cant_salas + 1) {
            printf("Nro de sala invalida!\n");
        } else if (msg.op.info_salas.asientos_por_sala[sala - 1] <= 0) {
            printf("Sala llena!\n");
        } else {
            salaElegida = true;
            sala = sala - 1; // La sala 1 en realidad es la 0 ...
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

    // TODO LEER DE LA MEMORIA COMPARTIDA LOS ASIENTOS QUE HAY
    int asientos[MAX_ASIENTOS_RESERVADOS], asiento, cantAsientos = 0;
    bool finEleccionAsientos = false;
    do {
        printf("\nElija un asiento (-1 para finalizar eleccion)(el primer asiento es 0): ");
        scanf("%d", &asiento);
        if (asiento == -1) {
            if (cantAsientos > 0) {
                finEleccionAsientos = true;
            } else {
                printf("Debe elegir al menos un asiento!\n");
            }
        } else if (asientos >= 0 && msg.op.info_asientos.asiento_habilitado[asiento] == DISPONIBLE) {
            asientos[cantAsientos++] = asiento;
            printf("Asiento %i agregado\n", asiento);
            if (cantAsientos == MAX_ASIENTOS_RESERVADOS) {
                finEleccionAsientos = true;
            }
        } else {
            printf("Nro de asiento invalido!\n");
        }
    } while (!finEleccionAsientos);

    elegir_asientos(asientos, cantAsientos, sala);

    /******* RECIBIR INFO RESERVA *********/

    msg = recibir_info_reserva();
    printf("\nSe reservaron los siguientes asientos:");
    for (int i = 0; i < msg.op.info_reserva.cant_reservados; i++) {
        printf(" %i", msg.op.info_reserva.asientos_reservados[i]);
    }
    printf("\n");

    /******* CONFIRMAR RESERVA *********/

    bool aceptarReserva = true; // TODO Harcodeo de aceptar. capaz hay q contemplar el caso q no aceptamos (ver una vez q terminemos lo otro)
    confirmar_reserva(aceptarReserva);

    /******* RECIBIR INFO PAGO *********/

    msg = recibir_info_pago();
    printf("\nSu total a abonar es: $%d\n", msg.op.info_pago.precio);

    /******* CONFIRMAR RESERVA *********/
    printf("\nPresione enter para enviar el pago...");
    getchar();
    pagar(msg.op.info_pago.precio);

    /******* RECIBIR PAGO ACEPTADO *********/

    recibir_pago_ok();
    printf("\nPago aceptado. Gracias por operar con red SARASA\n");

    liberarYSalir();

    return 0;
}
