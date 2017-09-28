#include "../../common/canal.h"

int main() {
    // proc -> el proceso que esta en el medio, entre mom cliente y mom cine
    // CREO QUE NO HACE NADA MÁS ESTE PROCESO ES UN PASAMANOS...
    canal *canal_cli_mom = canal_crear(mom, proc);
    canal *canal_mom_proc = canal_crear(mom, proc);
    while (true) {
        mensaje_t msg;
        canal_recibir(canal_cli_mom, msg, 0); // RECIBE MJE DEL CLIENTE
        canal_enviar(canal_mom_proc, msg); // LO MANDA AL PROC QUE A SU VEZ LO MANDA AL SERVER
        canal_recibir(canal_mom_proc, msg, 0); // RECIBE LA RTA
        canal_enviar(canal_cli_mom, msg); // MANDA LA RTA AL CLIENTE
    }
}