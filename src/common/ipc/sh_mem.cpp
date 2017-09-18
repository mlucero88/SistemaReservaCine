#include "sh_mem.h"

#include <cstring>
#include <sys/shm.h>

int sh_mem_create(int n, shmem_data* &mem_ptr) {
	key_t key = ftok("/media", n);
	if (key != -1) {
		int id = shmget(key, sizeof(shmem_data), 0666 | IPC_CREAT | IPC_EXCL);
		if (id != -1) {
			mem_ptr = static_cast<shmem_data*>(shmat(id, NULL, 0));
			if(mem_ptr != NULL) {
				memset(mem_ptr, 0, sizeof(shmem_data));
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

shmem_data* sh_mem_get(int n) {
	key_t key = ftok("/media", n);
	if (key != -1) {
		int id = shmget(key, sizeof(shmem_data), 0666);
		if (id != -1) {
			return static_cast<shmem_data*>(shmat(id, NULL, 0));
		}
	}
	return NULL;
}

void sh_mem_release(shmem_data* mem_ptr) {
	shmdt(static_cast<void*>(mem_ptr));
}
