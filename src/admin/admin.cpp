#include <random>
#include <cstring>
#include <cstdlib>

#include "../common/constantes.h"
#include "../common/operaciones.h"
#include "../common/color_print.h"
#include "../common/ipc/msg_queue.h"

#define ADMIN_LOG(fmt, ...) FPRINTF(stdout, KGRN, fmt, ##__VA_ARGS__)

void mostrar_asientos(int n_sala, int asientos_salas[MAX_SALAS][MAX_ASIENTOS]) {
	ADMIN_LOG("\nHay %i asientos en total en la sala\n", n_sala + 1);
    for (int j = 0; j < MAX_ASIENTOS; j++) {
    	ADMIN_LOG("%c\n", asientos_salas[n_sala][j] == DISPONIBLE ? 'O' : 'X');
    }
    ADMIN_LOG("\n");
}

void cancelar_reserva(int reservas[MAX_SALAS][MAX_ASIENTOS], int asientos_salas[MAX_SALAS][MAX_ASIENTOS], int cli_id,
                       int n_sala, int n_asientos_salas[MAX_SALAS]) {
	ADMIN_LOG("Cancelando la reserva del cliente %i\n", cli_id);
    for (int i = 0; i < MAX_ASIENTOS; ++i) {
        if (reservas[n_sala][i] == cli_id) {
            reservas[n_sala][i] = -1;		// Libre
            asientos_salas[n_sala][i] = DISPONIBLE;
            n_asientos_salas[n_sala]++;
        }
    }
}

void notificar_clientes(int q_admin_cliente, int nro_sala, int asientos_salas[MAX_SALAS][MAX_ASIENTOS],
                        int n_asientos_salas[MAX_SALAS], int salas_clientes[MAX_SALAS][MAX_CLIENTES]) {
    // Si se pudo reservar/liberar alguno de los asientos, les aviso a los clientes que estaban mirando esa sala
    mensaje_t msg;
    msg.tipo = INFORMAR_ASIENTOS;
    msg.op.info_asientos.nro_sala = nro_sala;
    msg.op.info_asientos.cant_asientos = n_asientos_salas[nro_sala];
    memcpy(msg.op.info_asientos.asiento_habilitado, asientos_salas[nro_sala], MAX_ASIENTOS * sizeof(int));
    ADMIN_LOG("Notificando clientes...\n");
    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (salas_clientes[nro_sala][i] != 0) {
            printf("Notificando cliente %i\n", salas_clientes[nro_sala][i]);
            // 0 si no hay cliente en esa posicion, si no, es el pid del cliente, != 0
            msg.mtype = salas_clientes[nro_sala][i];
            msg_queue_send(q_admin_cliente, &msg);
        }
    }
}

void quitar_cliente_sistema(int cli_id, int salas_clientes[MAX_SALAS][MAX_CLIENTES], int n_sala = 0) {
    for (; n_sala < MAX_SALAS; ++n_sala) {
    	for (int j = 0; j < MAX_CLIENTES; ++j) {
    		if (salas_clientes[n_sala][j] == cli_id) {
    			ADMIN_LOG("LOGOUT cliente %i\n", cli_id);
    			salas_clientes[n_sala][j] = 0;
    			break;
    		}
    	}
    }
}

void cargar_datos(int n_salas, int n_asientos_salas[MAX_SALAS], int asientos_salas[MAX_SALAS][MAX_ASIENTOS]) {
    for (int i = 0; i < n_salas; i++) {
        int n_asientos = rand() % MAX_ASIENTOS;
        n_asientos_salas[i] = n_asientos;
        for (int j = 0; j < n_asientos; j++) {
            asientos_salas[i][j] = DISPONIBLE;
        }
        for (int j = n_asientos; j < MAX_ASIENTOS; j++) {
            asientos_salas[i][j] = NO_DISPONIBLE;
        }
    }
    for (int i = n_salas; i < MAX_SALAS; i++) {
        n_asientos_salas[i] = 0;
        for (int j = 0; j < MAX_ASIENTOS; j++) {
            asientos_salas[i][j] = NO_DISPONIBLE;
        }
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int n_salas = 1 + (rand() % MAX_SALAS);			// Cantidad de salas que hay en total
    int n_asientos_salas[MAX_SALAS];				// Cantidad de asientos que hay en cada sala
    int asientos_salas[MAX_SALAS][MAX_ASIENTOS];	// Asientos disponibles/ocupados en cada sala
    int salas_clientes[MAX_SALAS][MAX_CLIENTES];
    int reservas[MAX_SALAS][MAX_ASIENTOS]; 			// En cada posición está el id del cliente que reservó ese asiento

    // Lleno con ceros -> no hay clientes
    memset(salas_clientes, 0, sizeof(int) * MAX_SALAS * MAX_CLIENTES);
    memset(reservas, 0, sizeof(int) * MAX_SALAS * MAX_ASIENTOS);

    cargar_datos(n_salas, n_asientos_salas, asientos_salas);

    ADMIN_LOG("Cantidad de salas: %i\n", n_salas);

    for (int i = 0; i < MAX_SALAS; i++) {
    	ADMIN_LOG("SALA %i -> %i\n", i + 1, n_asientos_salas[i]);
        for (int j = 0; j < MAX_ASIENTOS; j++) {
        	ADMIN_LOG("%c", asientos_salas[i][j] == DISPONIBLE ? 'O' : 'X');
        }
        ADMIN_LOG("\n");
    }

    int q_cine_snd = msg_queue_get(Q_ADMIN_CINE);
    int q_cine_rcv = msg_queue_get(Q_CINE_ADMIN);
    if (q_cine_snd == -1 ||  q_cine_rcv == -1) {
    	ADMIN_LOG("Error al crear canal de comunicacion entre admin y cine\n");
        exit(1);
    }


    int q_cliente_snd = msg_queue_get(Q_ADMIN_CLI);
    if (q_cliente_snd == -1) {
    	ADMIN_LOG("Error al crear canal de comunicacion entre admin y cliente\n");
        exit(1);
    }

    while (true) {
    	mensaje_t msg;
    	int cli_id;

    	ADMIN_LOG("Esperando mensaje...\n");

    	if (msg_queue_receive(q_cine_rcv, 0, &msg)) {
    		ADMIN_LOG("Recibí mensaje con tipo %i y mtype %i\n", msg.tipo, msg.mtype);
    		cli_id = msg.mtype;

    		switch(msg.tipo) {

    		case INFORMAR_SALAS: {
    			ADMIN_LOG("INFORMAR_SALAS para cliente %i\n", cli_id);
    		    memcpy(msg.op.info_salas.asientos_por_sala, n_asientos_salas, MAX_SALAS * sizeof(int));
    		    msg.op.info_salas.cant_salas = n_salas;
    		    ADMIN_LOG("Enviando INFORMAR_SALAS a cliente %i\n", cli_id);
    		    msg_queue_send(q_cine_snd, &msg);
    			break;
    		}

    		case ELEGIR_SALA: {
    		    int nro_sala = msg.op.elegir_sala.nro_sala;
    			ADMIN_LOG("INFORMAR_ASIENTOS sala %i para cliente %i\n", nro_sala + 1, cli_id);

   		        // PONGO EL NRO DE CLIENTE EN LA PRIMERA POSICIÓN LIBRE
    		    for (int i = 0; i < MAX_CLIENTES; ++i) {
    		        if (salas_clientes[nro_sala][i] == 0) {
    		            salas_clientes[nro_sala][i] = cli_id;
    		            break;
    		        }
    		    }
    		    ADMIN_LOG("Cliente %i en la sala %i\n", cli_id, nro_sala + 1);
    		    mostrar_asientos(nro_sala, asientos_salas);

    		    msg.tipo = INFORMAR_ASIENTOS;
    		    memcpy(msg.op.info_asientos.asiento_habilitado, asientos_salas[nro_sala], MAX_ASIENTOS * sizeof(int));
    		    msg.op.info_asientos.cant_asientos = n_asientos_salas[nro_sala];
    		    msg.op.info_asientos.nro_sala = nro_sala;
    		    ADMIN_LOG("Enviando INFORMAR_ASIENTOS a cliente %i\n", cli_id);
    		    msg_queue_send(q_cine_snd, &msg);
    			break;
    		}

    		case ELEGIR_ASIENTOS: {
    			ADMIN_LOG("INFORMAR_RESERVA para cliente %i\n", cli_id);
    		    mensaje_t respuesta;
    		    respuesta.tipo = INFORMAR_RESERVA;
    		    respuesta.mtype = cli_id;
    		    memset(respuesta.op.info_reserva.asientos_reservados, 0, MAX_ASIENTOS_RESERVADOS * sizeof(int));

    		    const int nro_sala = msg.op.elegir_asientos.nro_sala;
    		    const int nro_asientos_pedidos = msg.op.elegir_asientos.cant_elegidos;
    		    int asientos_reservados = 0;

    		    ADMIN_LOG("Veo si se pueden reservar %i asientos en la sala %i\n", nro_asientos_pedidos, nro_sala + 1);
    		    for (int i = 0; i < nro_asientos_pedidos; i++) {
    		        int nro_asiento = msg.op.elegir_asientos.asientos_elegidos[i];
    		        if (asientos_salas[nro_sala][nro_asiento] == DISPONIBLE) {
    		        	respuesta.op.info_reserva.asientos_reservados[i] = 1;
    		            asientos_salas[nro_sala][nro_asiento] = RESERVADO;
    		            reservas[nro_sala][nro_asiento] = cli_id;
    		            asientos_reservados++;
    		            ADMIN_LOG("Un asiento reservado: %i para el cliente %i\n", nro_asiento, cli_id);
    		        }
    		    }
    			ADMIN_LOG("Se reservaron %i asientos para el cliente %i\n", asientos_reservados, cli_id);
    		    n_asientos_salas[nro_sala] -= asientos_reservados; // Descuento los asientos reservados de la sala
    		    respuesta.op.info_reserva.cant_reservados = asientos_reservados;

    		    ADMIN_LOG("Enviando INFORMAR_RESERVA a cliente %i\n", cli_id);
    		    msg_queue_send(q_cine_snd, &respuesta);

    			if (asientos_reservados > 0) {
    				notificar_clientes(q_cliente_snd, nro_sala, asientos_salas, n_asientos_salas, salas_clientes);
    			}
    			break;
    		}

    		case CONFIRMAR_RESERVA: {
    			if(msg.op.confirmar_reserva.reserva_confirmada) {
    				ADMIN_LOG("Reserva confirmada por cliente %i\n", cli_id);

    				msg.tipo = INFORMAR_PAGO;
    				msg.op.info_pago.precio = 500;
    				ADMIN_LOG("Enviando INFORMAR_PAGO a cliente %i\n", cli_id);
    				msg_queue_send(q_cine_snd, &msg);
    			}
    			else {
    				ADMIN_LOG("Reserva cancelada por cliente %i\n", cli_id);
        			quitar_cliente_sistema(cli_id, salas_clientes);
    			}
    			break;
    		}

    		case PAGAR: {
    			msg.tipo = PAGO_OK;
    			msg.op.pago_ok.pago_ok = 500;
    			ADMIN_LOG("Enviando PAGO_OK a cliente %i\n", cli_id);
    		    msg_queue_send(q_cine_snd, &msg);

    			quitar_cliente_sistema(cli_id, salas_clientes);
    		    break;
    		}

    		case TIMEOUT: {
    			// Cancelo la reserva que había hecho el cliente, si es que había alguna y le aviso a los demás clientes
    			// todo: no hay q sacar aca tb al cliente de la sala (salas_clientes)?
    			int cliente = msg.op.timeout.cli_id;
    			int n_sala = msg.op.timeout.n_sala;

    			cancelar_reserva(reservas, asientos_salas, cliente, n_sala, n_asientos_salas);
    			quitar_cliente_sistema(cliente, salas_clientes, n_sala);
    			notificar_clientes(q_cliente_snd, n_sala, asientos_salas, n_asientos_salas, salas_clientes);
    			break;
    		}
    		default: {
    			break;
    		}
    		}
    	}
    }
}
