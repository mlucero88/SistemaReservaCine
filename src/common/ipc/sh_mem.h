#ifndef COMMON_IPC_SH_MEM_H_
#define COMMON_IPC_SH_MEM_H_

#include "../constantes.h"

struct shmem_data {
    int asientos[MAX_ASIENTOS];
    int cantidad;
    bool dirty;
};

struct shmem;

/* Utilizadas por proceso maestro (q crea y destruye la memoria) */
shmem *sh_mem_create(int n);

void sh_mem_destroy(shmem *mem);
/**************/

/* Utilizadas por procesos esclavos (q no crean ni destruyen la memoria) */
shmem *sh_mem_get(int n);

void sh_mem_release(shmem *mem);
/**************/

/* Locked read/write */
void sh_mem_write(shmem *mem, const shmem_data *data);

void sh_mem_read(shmem *mem, shmem_data *data);


#endif /* COMMON_IPC_SH_MEM_H_ */
