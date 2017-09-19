#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <signal.h>

#include "../common/canal.h"
#include "../common/ipc/sh_mem.h"
#include "../common/color_print.h"
#include "../common/operaciones.h"

#define CLI_LOG asyncli_log
#define CLI_PRINTF(fmt, ...) FPRINTF(CLI_LOG, RED, fmt, ##__VA_ARGS__)

int shmemId;
canal *canal_cli_admin = nullptr;
shmem_data *sharedData = nullptr;
FILE *asyncli_log = nullptr;

void liberarYSalir() {
    CLI_PRINTF("Preparando para salir...");
    if (sharedData != nullptr) {
        sh_mem_release(sharedData);
        CLI_PRINTF("Memoria compartida liberada");
    }
    if (canal_cli_admin != nullptr) {
        canal_destruir(canal_cli_admin);
        CLI_PRINTF("Canal destruido");
    }

    CLI_PRINTF("*** Aplicacion cerrada ***");
    fclose(asyncli_log);
    exit(1);
}

void sighandler(int signum) {
    if (signum == SIGINT) {
        liberarYSalir();
    }
}

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
	char logName[64];
	sprintf(logName, "./asyncli_%i" ".log", pid);
	asyncli_log = fopen(logName, "w");
    signal(SIGINT, sighandler);


    entidad_t cliente = {.proceso = entidad_t::CLIENTE, .pid = pid};
    entidad_t admin = {.proceso = entidad_t::ADMIN, .pid = -1};
    int cli_pid = atoi(argv[1]);
    mensaje_t msg;

    canal_cli_admin = canal_crear(cliente, admin);
    if (canal_cli_admin == NULL) {
        CLI_PRINTF("Error al crear canal de comunicacion entre cliente y admin: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("Canal de comunicacion entre cliente y admin creado");

    sharedData = sh_mem_get(cli_pid);
    if (sharedData == NULL) {
        CLI_PRINTF("Error al obtener memoria compartida entre cliente y cliente_asyn: %s", strerror(errno));
        liberarYSalir();
    }
    CLI_PRINTF("Memoria compartida entre cliente y cliente_asyn obtenida");

    while (true) {
        if (!canal_recibir(canal_cli_admin, msg, cli_pid)) {
            CLI_PRINTF("Error al recibir mensaje de INFORMAR_ASIENTOS: %s", strerror(errno));
            liberarYSalir();
        }
        CLI_PRINTF("Recibida actualizacion de sala %i", msg.op.info_asientos.nro_sala + 1);
        // TODO ***** TOMAR LOCK *****
        sharedData->cantidad = msg.op.info_asientos.cant_asientos;
        memcpy(sharedData->asientos, msg.op.info_asientos.asiento_habilitado, sizeof(int) * sharedData->cantidad);

        for (int i = 0; i < MAX_ASIENTOS; i++) {
            CLI_PRINTF("(%i)", msg.op.info_asientos.asiento_habilitado[i]);
        }
        sharedData->dirty = true;
        // TODO ***** LIBERAR LOCK *****
    }
}
