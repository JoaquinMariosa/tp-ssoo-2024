#include "../include/memoria-entradasalida.h"

void conexion_memoria_entradasalida() {
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_entradasalida);
		switch (cod_op) {
		case MENSAJE:
			log_info(memoria_logger, "Mensaje recibido con el numero");
			break;
		case PAQUETE:
			break;
		case RECIBIR_NOMBRE_IO:
			t_interfaz* interfaz = malloc(sizeof(t_interfaz));
			uint8_t MAX_LENGTH = 128;
			char* nombre = malloc(MAX_LENGTH);
			char* nombre_recibido = malloc(MAX_LENGTH);
			char* tipo = malloc(MAX_LENGTH);
			char* tipo_recibido = malloc(MAX_LENGTH);
			
			// Recibo nombre de la interfaz conectada
			if(!recv_interfaz(fd_entradasalida, nombre_recibido, tipo_recibido)) {
				log_error(memoria_logger, "Hubo un erro al recibir el nombre de la interfaz.");
			}
			strcpy(nombre, nombre_recibido);
			strcpy(tipo, tipo_recibido);

			// Guarda en una lista por cada interfaz (t_interfaz)
			interfaz->nombre = nombre;
			interfaz->socket = fd_entradasalida;
			interfaz->tipo = tipo;
			list_add(listaInterfaces, interfaz);

			// Crear hilo de conexion de cada interfaz
            pthread_t memoria_interfaces;
            int* fd_interfaz = malloc(sizeof(int));
            *fd_interfaz = interfaz->socket;
            pthread_create(&memoria_interfaces, NULL, (void *)conexion_memoria_interfaces, (void *)fd_interfaz);
            pthread_detach(memoria_interfaces);
			
			// Liberar memoria
			free(nombre_recibido);
			free(tipo_recibido);

			// Termina este servidor y se va al de la interfaz en especifico
			control = 0;
			break;
		case -1:
			log_error(memoria_logger, "El IO se desconecto. Terminando servidor");
			control = 0;
            break;
		default:
			log_warning(memoria_logger, "Operacion desconocida.");
			break;
		}
	}
}