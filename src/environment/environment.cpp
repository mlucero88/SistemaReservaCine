#include <cerrno>
#include <cstring>
#include <iostream>

#include "../common/ipc/msg_queue.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " --[start|stop]" << std::endl;
        return -1;
    }

    if (std::strcmp(argv[1], "--start") == 0) {
        bool success = true;

        /* Intento crear todas. Si alguna falla, sigo intentando con las demas */
        if (msg_queue_create(Q_ADMIN_CINE) == -1) {
            std::cerr << "Error al crear msg_q ADMIN_CINE: " << strerror(errno) << std::endl;
            success = false;
        }
        if (msg_queue_create(Q_CINE_ADMIN) == -1) {
            std::cerr << "Error al crear msg_q CINE_ADMIN: " << strerror(errno) << std::endl;
            success = false;
        }
        if (msg_queue_create(Q_CINE_CLI_A) == -1) {
            std::cerr << "Error al crear msg_q CINE_CLI: " << strerror(errno) << std::endl;
            success = false;
        }
        if (msg_queue_create(Q_CLI_CINE_A) == -1) {
            std::cerr << "Error al crear msg_q CLI_CINE: " << strerror(errno) << std::endl;
            success = false;
        }
        if (msg_queue_create(Q_CINE_CLI_B) == -1) {
            std::cerr << "Error al crear msg_q CINE_CLI: " << strerror(errno) << std::endl;
            success = false;
        }
        if (msg_queue_create(Q_CLI_CINE_B) == -1) {
            std::cerr << "Error al crear msg_q CLI_CINE: " << strerror(errno) << std::endl;
            success = false;
        }
        if (msg_queue_create(Q_ADMIN_CLI) == -1) {
            std::cerr << "Error al crear msg_q ADMIN_CLI: " << strerror(errno) << std::endl;
            success = false;
        }
        if (msg_queue_create(Q_CLI_MOM) == -1) {
            std::cerr << "Error al crear msg_q CLI_MOM: " << strerror(errno) << std::endl;
            success = false;
        }
        if (msg_queue_create(Q_MOM_CLI) == -1) {
            std::cerr << "Error al crear msg_q MOM_CLI: " << strerror(errno) << std::endl;
            success = false;
        }

        if (!success) {
            std::cout << "Se encontraron errores al crear algunos IPC (posiblemente ya existian)\n";
            std::cout << "Se recomienda ejecutar --stop y probar otra vez" << std::endl;
            return -2;
        }

        std::cout << "Se crearon todos los IPC" << std::endl;
        return 0;
    }

    if (std::strcmp(argv[1], "--stop") == 0) {
        msg_queue_destroy(msg_queue_get(Q_ADMIN_CINE));
        msg_queue_destroy(msg_queue_get(Q_CINE_ADMIN));
        msg_queue_destroy(msg_queue_get(Q_CINE_CLI_A));
        msg_queue_destroy(msg_queue_get(Q_CLI_CINE_A));
        msg_queue_destroy(msg_queue_get(Q_CINE_CLI_B));
        msg_queue_destroy(msg_queue_get(Q_CLI_CINE_B));
        msg_queue_destroy(msg_queue_get(Q_ADMIN_CLI));
        msg_queue_destroy(msg_queue_get(Q_CLI_MOM));
        msg_queue_destroy(msg_queue_get(Q_MOM_CLI));

        std::cout << "Se liberaron todos los IPC" << std::endl;
        return 0;
    }

    std::cout << "Usage: " << argv[0] << " --[start|stop]" << std::endl;
    return -1;
}
