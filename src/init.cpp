#include "msg.h"
#include "commons.h"

int main(){
    msg_create(Q_ADMIN_CINE);
    msg_create(Q_CINE_ADMIN);
    msg_create(Q_CINE_CLI);
    msg_create(Q_CLI_CINE);
}