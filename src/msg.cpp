#include "msg.h"

int msg_create(char n, int flags) {
    key_t key_1 = ftok("./README.md", n);

    if (key_1 == -1) {
        return -1;
    }

    int msg_1 = msgget(key_1, 0777 | IPC_CREAT | IPC_EXCL);

    return msg_1;
}

int msg_get(char n, int flags) {
    key_t key_1 = ftok("./README.md", n);

    if (key_1 == -1) {
        return -1;
    }

    int msg_1 = msgget(key_1, 0777);

    return msg_1;
}

int msg_destroy(int id){
    return msgctl(id,IPC_RMID,NULL);
}

int msg_recv()