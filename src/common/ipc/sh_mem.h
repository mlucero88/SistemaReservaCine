#ifndef COMMON_IPC_SH_MEM_H_
#define COMMON_IPC_SH_MEM_H_

#include <cstddef>

int sh_mem_create(int n, size_t memsize, void **mem_ptr);

int sh_mem_destroy(int shmem_id);

/* Obtiene un puntero a la memoria compartida (debe estar creada). Si retorna NULL, la operacion fallo (consultar errno) */
void* sh_mem_get(int n, size_t memsize);

/* Libera el puntero a la memoria compartida obtenida en sh_mem_get */
void sh_mem_release(void* mem_ptr);

#endif /* COMMON_IPC_SH_MEM_H_ */
