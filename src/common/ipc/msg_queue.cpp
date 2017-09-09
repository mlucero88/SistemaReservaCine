#include <cstring>
#include <sys/msg.h>

#include "msg_queue.h"

int msg_queue_create(msg_queue_direction n) {
	key_t key = ftok("/media", n);
	return (key != -1) ? msgget(key, 0666 | IPC_CREAT | IPC_EXCL) : key;
}

int msg_queue_destroy(int id) {
	return msgctl(id, IPC_RMID, NULL);
}

int msg_queue_get(msg_queue_direction n) {
	key_t key = ftok("/media", n);
	return (key != -1) ? msgget(key, 0666) : key;
}

bool msg_queue_send(int q_id, const mensaje_t *mensaje) {
	return (msgsnd(q_id, static_cast<const void *>(mensaje), sizeof(mensaje_t) - sizeof(long), 0) == 0);
}

bool msg_queue_receive(int q_id, long msg_type, mensaje_t *mensaje) {
	return (msgrcv(q_id, static_cast<void *>(mensaje), sizeof(mensaje_t) - sizeof(long), msg_type, 0) >= 0);
}
