#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Uso: $0 [cliente|cine]"
	exit 1
fi

if [ "$1" == "cliente" ]; then
	ARRAYNAME="BINARIOS_CLI"
elif [ "$1" == "cine" ]; then
	ARRAYNAME="BINARIOS_CINE"
else
	echo "Uso: $0 [cliente|cine]"
	exit 1
fi

PROJ_DIR=`dirname $(readlink -f $0)`
BINARIOS_CLI=("environment" "mom" "cliente" "socket_adapter_cliente")
BINARIOS_CINE=("environment" "cine_login" "cine" "admin" "socket_adapter_cine_server" "socket_adapter_cine")
SRC_DIR=${PROJ_DIR}/src
COMMON_DIR=${SRC_DIR}/common

bins="${ARRAYNAME}[@]"

mkdir -p ${PROJ_DIR}/bin

echo "*** LIMPIANDO BINARIOS... ***"

for bin in "${!bins}"; do
	rm ${PROJ_DIR}/bin/$bin 2> /dev/null
done

for bin in "${!bins}"; do
	echo "*** COMPILANDO ${bin}... ***"

	case "${bin}" in
		environment)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		mom)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/ipc/sock.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
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
		socket_adapter_cine_server)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/ipc/sock.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		socket_adapter_cine)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/ipc/sock.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
		socket_adapter_cliente)
			g++ -std=c++11 -g3 -O0 -I${SRC_DIR} ${COMMON_DIR}/ipc/msg_queue.cpp ${COMMON_DIR}/ipc/sock.cpp ${SRC_DIR}/${bin}/${bin}.cpp -o ${PROJ_DIR}/bin/${bin}
			;;
	esac
	
	if [ $? -eq 0 ]; then
		echo "*** ${bin} compilado exitosamente ***"
	else
		echo "*** Error al compilar ${bin} ***"
	fi
done
