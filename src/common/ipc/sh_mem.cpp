#include "sh_mem.h"

#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>

struct shmem {
	shmem_data *data;
	struct flock lock;
	int id;
	int lockFd;
};

shmem* sh_mem_create(int n) {
	key_t key = ftok("/media", n);
	if (key != -1) {
		int id = shmget(key, sizeof(shmem_data), 0666 | IPC_CREAT | IPC_EXCL);
		if (id != -1) {
			shmem_data* mem_ptr = static_cast<shmem_data*>(shmat(id, NULL, 0));
			if (mem_ptr != NULL) {
				memset(mem_ptr, 0, sizeof(shmem_data));
				shmem *mem = (shmem *) malloc(sizeof(shmem));
				mem->id = id;
				mem->data = mem_ptr;
				memset(&mem->lock, 0, sizeof(flock));
				mem->lock.l_type = F_WRLCK;
				mem->lockFd = open("../compilar.sh", O_WRONLY);
				return mem;
			}
			shmctl(id, IPC_RMID, NULL);
		}
	}
	return NULL;
}

void sh_mem_destroy(shmem* mem) {
	if (mem != NULL) {
		shmdt(static_cast<void*>(mem->data));
		shmctl(mem->id, IPC_RMID, NULL);
		free(mem);
	}
}

shmem* sh_mem_get(int n) {
	key_t key = ftok("/media", n);
	if (key != -1) {
		int id = shmget(key, sizeof(shmem_data), 0666);
		if (id != -1) {
			shmem_data* mem_ptr = static_cast<shmem_data*>(shmat(id, NULL, 0));
			if (mem_ptr != NULL) {
				shmem *mem = (shmem *) malloc(sizeof(shmem));
				mem->id = id;
				mem->data = mem_ptr;
				return mem;
			}
		}
	}
	return NULL;
}

void sh_mem_release(shmem* mem) {
	if (mem != NULL) {
		shmdt(static_cast<void*>(mem->data));
		free(mem);
	}
}

void sh_mem_write(shmem* mem, const shmem_data *data) {
	mem->lock.l_type = F_WRLCK;
	fcntl(mem->lockFd, F_SETLKW, &mem->lock);
	memcpy(mem->data, data, sizeof(shmem_data));
	mem->lock.l_type = F_UNLCK;
	fcntl(mem->lockFd, F_SETLK, &mem->lock);
}

void sh_mem_read(shmem* mem, shmem_data *data) {
	mem->lock.l_type = F_RDLCK;
	fcntl(mem->lockFd, F_SETLKW, &mem->lock);
	memcpy(data, mem->data, sizeof(shmem_data));
	mem->lock.l_type = F_UNLCK;
	fcntl(mem->lockFd, F_SETLK, &mem->lock);
}
