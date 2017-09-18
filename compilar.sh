#!/bin/bash

PROJ_DIR=`dirname $(readlink -f $0)`
BINARIOS=("environment" "cliente" "cliente_asyn" "cine_login" "cine" "admin")
SRC_DIR=${PROJ_DIR}/src
COMMON_DIR=${SRC_DIR}/common

mkdir -p ${PROJ_DIR}/bin

echo "*** LIMPIANDO BINARIOS... ***"

for bin in ${BINARIOS[@]}; do
	rm ${PROJ_DIR}/bin/$bin 2> /dev/null
done

for bin in ${BINARIOS[@]}; do
	echo "*** COMPILANDO ${bin}... ***"

	case "${bin}" in
		environment)
			g++ -std=c++11 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${SRC_DIR}/${bin}/main.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		cliente)
			g++ -std=c++11 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/ipc/sh_mem.cpp ${COMMON_DIR}/canal.cpp ${SRC_DIR}/cliente/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		cliente_asyn)
			g++ -std=c++11 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/ipc/sh_mem.cpp ${COMMON_DIR}/canal.cpp ${SRC_DIR}/cliente/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		cine_login)
			g++ -std=c++11 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/canal.cpp ${SRC_DIR}/${bin}/main.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		cine)
			g++ -std=c++11 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/canal.cpp ${SRC_DIR}/${bin}/main.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		admin)
			g++ -std=c++11 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/canal.cpp ${SRC_DIR}/${bin}/main.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
	esac
	
	if [ $? -eq 0 ]; then
		echo "*** ${bin} compilado exitosamente ***"
	else
		echo "*** Error al compilar ${bin} ***"
	fi
done
