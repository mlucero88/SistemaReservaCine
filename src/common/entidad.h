#ifndef COMMON_ENTIDAD_H_
#define COMMON_ENTIDAD_H_

#include <unistd.h>

struct entidad_t {
	enum proceso_t {
		CINE, CLIENTE, ADMIN
	} proceso;

	pid_t pid;
	// extension a futuro
};

#endif /* COMMON_ENTIDAD_H_ */
