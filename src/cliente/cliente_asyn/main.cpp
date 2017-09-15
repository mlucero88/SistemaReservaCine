#include "../../common/canal.h"

int main() {
    canal *canal_admin_cliente = canal_crear();
    while (true) {
        mensaje_t msg;
        canal_recibir(canal_admin_cliente, msg, cli_id);
        // compartir la info con el cliente en memoria compartida

    }
}