#ifndef COMMON_IPC_SH_MEM_H_
#define COMMON_IPC_SH_MEM_H_

#include "../constantes.h"

struct shmem_data {
    int asientos[MAX_ASIENTOS];
    int cantidad;
    bool dirty;
};

int sh_mem_create(int n, shmem_data* &mem_ptr);

int sh_mem_destroy(int shmem_id);

/* Obtiene un puntero a la memoria compartida (debe estar creada). Si retorna NULL, la operacion fallo (consultar errno) */
shmem_data* sh_mem_get(int n);

/* Libera el puntero a la memoria compartida obtenida en sh_mem_get */
void sh_mem_release(shmem_data* mem_ptr);

#endif /* COMMON_IPC_SH_MEM_H_ */
