#include "../include/memoria-kernel.h"

t_list* instrucciones;
t_list* instrucciones_por_proceso;

void conexion_memoria_kernel() {
	instrucciones_por_proceso = list_create();
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op) {
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case INICIAR_PROCESO:
			uint32_t MAX_LENGTH = 128;
			uint8_t pid;
			char* path = malloc(MAX_LENGTH);
			char* path_recivido = malloc(MAX_LENGTH);
			char* path_concatenado = malloc(MAX_LENGTH);

			// Recibe el path desde el kernel
			if (!recv_iniciar_proceso(fd_kernel, &pid, path_recivido)) {
				log_error(memoria_logger, "Hubo un error al recibir INICIAR_PROCESO");
				free(path);
				free(path_recivido);
				free(path_concatenado);
				return;
			}

			strcpy(path, path_recivido);

			usleep(RETARDO_RESPUESTA * 1000);

			// Loguear el path concatenado
			log_info(memoria_logger, "Path recivido: %s", path);

			// Concatenar el PATH_INSTRUCCIONES con el path recibido
			snprintf(path_concatenado, MAX_LENGTH, "%s%s", PATH_INSTRUCCIONES, path);  // Concatena

			// Loguear el path concatenado
			log_info(memoria_logger, "Path concatenado: %s", path_concatenado);

			// Leer el archivo y obtener las instrucciones (con la función que mencionaste)
			instrucciones = leer_archivo(path_concatenado);

			// Listar segun el PID recibido
			list_add_in_index(instrucciones_por_proceso, pid, instrucciones);

			// Crear tabla de paginas vacia para cada proceso
            log_info(memoria_logger, "Creacion de Tabla de Paginas");
            log_info(memoria_logger, "PID: %d - Tamaño: %d", pid, 0);

			// Crear tabla de paginas para cada proceso
            t_list* tabla_paginas = list_create();
            list_add(tabla_paginas_por_proceso, tabla_paginas);

			// Liberar espacio
			free(path);
			free(path_recivido);
			free(path_concatenado);
            break;
		case FIN_PROCESO:
            uint8_t pid_fin;
			if (!recv_fin_proceso(fd_kernel, &pid_fin)) {
				log_error(memoria_logger, "Hubo un error al recibir el PID del proceso a finalizar");
				break;
			}

			log_info(memoria_logger, "Destruccion de Tabla de Paginas");

            // Liberar marcos
			if (pid_fin < list_size(tabla_paginas_por_proceso)) {
				t_list* tabla_paginas_borrar = list_get(tabla_paginas_por_proceso, pid_fin);
				if (tabla_paginas_borrar != NULL) {
            		log_info(memoria_logger, "PID: %d - Tamaño: %d", pid_fin, list_size(tabla_paginas_borrar));
					while (!list_is_empty(tabla_paginas_borrar)) {
						int* marco_asignado = list_remove(tabla_paginas_borrar, 0);
						if (marco_asignado != NULL) {
							liberar_marco(marcos_ocupados, *marco_asignado);
							free(marco_asignado); // Liberar la memoria del puntero
						}
					}
					list_destroy(tabla_paginas_borrar);
					// list_remove(tabla_paginas_por_proceso, pid_fin); // Revisar porque si lo dejo no anda
				}
			}
            break;
		case -1:
			log_error(memoria_logger, "El Kernel se desconecto. Terminando servidor");
			control = 0;
            break;
		default:
			log_warning(memoria_logger, "Operacion desconocida.");
			break;
		}
	}
}