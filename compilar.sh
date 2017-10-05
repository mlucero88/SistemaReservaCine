#!/bin/bash

PROJ_DIR=`dirname $(readlink -f $0)`
BINARIOS=("environment" "mom" "cliente" "cine_login" "cine" "admin")
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
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		mom)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		cliente)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${SRC_DIR}/${bin}/interfaz.cpp ${SRC_DIR}/${bin}/${bin}.cpp -lpthread -o ${PROJ_DIR}/bin/${bin}
			;;
		cine_login)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		cine)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		admin)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
	esac
	
	if [ $? -eq 0 ]; then
		echo "*** ${bin} compilado exitosamente ***"
	else
		echo "*** Error al compilar ${bin} ***"
	fi
done
