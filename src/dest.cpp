#include "msg.h"

int main() {
    int msg_1 = msg_get(Q_ADMIN_CINE);
    msg_destroy(msg_1);
    msg_1 = msg_get(Q_CINE_ADMIN);
    msg_destroy(msg_1);
    msg_1 = msg_get(Q_CINE_CLI);
    msg_destroy(msg_1);
    msg_1 = msg_get(Q_CLI_CINE);
    msg_destroy(msg_1);
}