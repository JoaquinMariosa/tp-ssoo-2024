#include "../include/entradasalida-memoria.h"

void conexion_entradasalida_memoria() {
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op) {
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case RECIBIR_VALOR_MEMORIA:
			void* datos;
			uint32_t length;
			uint32_t dir_fisica;
			if(!recv_valor_memoria(fd_memoria, &dir_fisica, &datos, &length)) {
				log_error(entradasalida_logger, "Hubo un error al recibir los datos de memoria");
			}

			if(strcmp(TIPO_INTERFAZ, "STDOUT") == 0) {
				char* datos_char = malloc(length);
				memcpy(datos_char, datos, length);

				// Muestra el string en pantalla
				log_info(entradasalida_logger, "%.*s", length, datos_char);

				// Avisa que ya no esta mas interrumpido el proceso
				send_fin_io(fd_kernel, pcb_stdout->pid, nombre_stdout, strlen(nombre_stdout) + 1);

				// Liberar memoria
				free(pcb_stdout->registros);
				free(pcb_stdout);
				free(nombre_stdout);
				free(nombre_stdout_recibido);
				free(datos_char);
				free(datos);
			} else if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0) {
				// Escribir en el bloques.dat los datos recibidos
				write_archivo(nombre_archivo, datos, length, ptr_archivo_write, bitmap_blocks);

				// Avisa que ya no esta mas interrumpido el proceso
				send_fin_io(fd_kernel, pcb_fs_write->pid, nombre_fs, strlen(nombre_fs) + 1);

				// Liberar memoria
				free(pcb_fs_write->registros);
				free(pcb_fs_write);
				free(nombre_fs);
				free(nombre_fs_recibido);
				free(nombre_archivo);
				free(nombre_archivo_recibido);
				free(datos);
			}

			pthread_mutex_unlock(&mutexIO);
			break;
		case ESCRITURA_OK:
			uint8_t escritura_ok;
			
			if(!recv_escritura_ok(fd_memoria, &escritura_ok)) {
				log_error(entradasalida_logger, "Error al recibir escritura OK");
			}

			if(escritura_ok == 1) {
				log_info(entradasalida_logger, "Escritura OK");
			} else {
				log_error(entradasalida_logger, "Error en la escritura");
			}

			if(strcmp(TIPO_INTERFAZ, "STDIN") == 0) {
				// Avisa que ya no esta mas interrumpido el proceso
				send_fin_io(fd_kernel, pcb_stdin->pid, nombre_stdin, strlen(nombre_stdin) + 1);

				// Liberar memoria
				free(pcb_stdin->registros);
				free(pcb_stdin);
				free(nombre_stdin);
				free(nombre_stdin_recibido);
				free(string);
			} else if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0) {
				// Avisa que ya no esta mas interrumpido el proceso
				send_fin_io(fd_kernel, pcb_fs_read->pid, nombre_fs, strlen(nombre_fs) + 1);

				// Liberar memoria
				free(pcb_fs_read->registros);
				free(pcb_fs_read);
				free(nombre_fs);
				free(nombre_fs_recibido);
				free(nombre_archivo);
				free(nombre_archivo_recibido);
			}

			pthread_mutex_unlock(&mutexIO);
			break;
		case -1:
			log_error(entradasalida_logger, "El servidor de Memoria no se encuentra activo.");
			control = 0;
            break;
		default:
			log_warning(entradasalida_logger, "Operacion desconocida.");
			break;
		}
	}
}