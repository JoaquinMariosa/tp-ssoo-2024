#include "../include/kernel-entradasalida.h"

void conexion_kernel_entradasalida() {
	uint8_t MAX_LENGTH = 128;
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_entradasalida);
		switch (cod_op) {
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case RECIBIR_NOMBRE_IO:
			t_interfaz* interfaz = malloc(sizeof(t_interfaz));
			char* nombre = malloc(MAX_LENGTH);
			char* nombre_recibido = malloc(MAX_LENGTH);
			char* tipo = malloc(MAX_LENGTH);
			char* tipo_recibido = malloc(MAX_LENGTH);
			
			// Recibo nombre de la interfaz conectada
			if(!recv_interfaz(fd_entradasalida, nombre_recibido, tipo_recibido)) {
				log_error(kernel_logger, "Hubo un erro al recibir el nombre de la interfaz.");
			}
			strcpy(nombre, nombre_recibido);
			strcpy(tipo, tipo_recibido);

			// Guarda en una lista por cada interfaz (t_interfaz)
			interfaz->nombre = nombre;
			interfaz->socket = fd_entradasalida;
			interfaz->tipo = tipo;
			list_add(listaInterfaces, interfaz);

			// Crear hilo de conexion de cada interfaz
            pthread_t kernel_interfaces;
            int* fd_interfaz = malloc(sizeof(int));
            *fd_interfaz = interfaz->socket;
            pthread_create(&kernel_interfaces, NULL, (void *)conexion_kernel_interfaces, (void *)fd_interfaz);
            pthread_detach(kernel_interfaces);
			
			// Liberar memoria
			free(nombre_recibido);
			free(tipo_recibido);

			// Termina este servidor y se va al de la interfaz en especifico
			control = 0;
			break;
		case -1:
			log_error(kernel_logger, "El IO se desconecto. Terminando servidor");
			control = 0;
            break;
		default:
			log_warning(kernel_logger, "Operacion desconocida.");
			break;
		}
	}
}