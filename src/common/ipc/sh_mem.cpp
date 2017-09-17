#include "sh_mem.h"

#include <cstring>
#include <sys/shm.h>

int sh_mem_create(int n, size_t memsize, void **mem_ptr) {
	key_t key = ftok("/media", n);
	if (key != -1) {
		int id = shmget(key, memsize, 0666 | IPC_CREAT | IPC_EXCL);
		if (id != -1) {
			*mem_ptr = shmat(id, NULL, 0);
			if(*mem_ptr != NULL) {
				return id;
			}
			shmctl(id, IPC_RMID, NULL);
		}
	}
	return -1;
}

int sh_mem_destroy(int shmem_id) {
	return shmctl(shmem_id, IPC_RMID, NULL);
}

int sh_mem_id_get(int n, size_t memsize) {
	key_t key = ftok("/media", n);
	return (key != -1) ? shmget(key, memsize, 0666) : key;
}

void* sh_mem_get(int n, size_t memsize) {
	key_t key = ftok("/media", n);
	if (key != -1) {
		int id = shmget(key, memsize, 0666);
		if (id != -1) {
			return shmat(id, NULL, 0);
		}
	}
	return NULL;
}

void sh_mem_release(void* mem_ptr) {
	shmdt(mem_ptr);
}
