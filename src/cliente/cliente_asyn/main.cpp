#include <iostream>
#include <sys/ipc.h>
#include <cstring>
#include "../../common/ipc/msg_queue.h"
#include "../../common/colors.h"
#include <sys/shm.h>

int main(int argc, char *argv[]) {

    printf("%sINICIO CLIENTE ASYN\n", KCYN);

    int q_admin_cliente = msg_queue_get(Q_ADMIN_CLI);
    if (q_admin_cliente == -1) {
        std::cerr << "Error al crear canal de comunicacion entre admin y cliente" << std::endl;
        exit(1);
    }
    mensaje_t msg;
    int cli_id = atoi(argv[1]);

    key_t key = ftok("/media", cli_id);
    int shmid = shmget(key, sizeof(int) * MAX_ASIENTOS, 0666);
    int *ptr = (int *) shmat(shmid, NULL, 0);

    while (true) {
        printf("%sESPERO MENSAJE CON MTYPE %i EN LA COLA%i%s\n", KCYN, cli_id, q_admin_cliente, KNRM);
        msg_queue_receive(q_admin_cliente, cli_id, &msg);
        printf("%sRECIBO ACTUALIZACION DE SALA%s\n", KCYN, KNRM);
        // Escribo la info recibida en la memoria compartida con el cliente posta
        // ***** ACÁ HAY QUE PONER UN LOCK, PARA QUE EL CLIENTE NO QUIERA LEER JUSTO CUANDO YO ESTOY ESCRIBIENDO, AUNQUE SERAÍ MUY RARO QUE PASE*****
        // ***** TOMAR LOCK *****
        memcpy(msg.op.info_asientos.asiento_habilitado, ptr, sizeof(int) * MAX_ASIENTOS); // No es atómica
        // ***** LIBERAR LOCK *****
    }
}