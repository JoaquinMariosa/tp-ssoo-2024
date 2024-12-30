#include "../include/entradasalida-kernel.h"

t_pcb* pcb_stdout;
t_pcb* pcb_stdin;
t_pcb* pcb_fs_write;
t_pcb* pcb_fs_read;
char* nombre_sleep;
char* nombre_sleep_recibido;
char* nombre_stdin;
char* nombre_stdin_recibido;
char* nombre_stdout;
char* nombre_stdout_recibido;
char* nombre_fs;
char* nombre_fs_recibido;
char* nombre_archivo;
char* nombre_archivo_recibido;
char* string;
uint32_t ptr_archivo_write;
uint32_t MAX_LENGTH;
pthread_mutex_t mutexIO = PTHREAD_MUTEX_INITIALIZER;

void conexion_entradasalida_kernel() {
	MAX_LENGTH = 128;
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op) {
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case IO_GEN_SLEEP:
			pthread_mutex_lock(&mutexIO);
			t_pcb* pcb_sleep = malloc(sizeof(t_pcb));
			pcb_sleep->registros = malloc(sizeof(t_registros));
			uint32_t unidades_de_trabajo;
			nombre_sleep = malloc(MAX_LENGTH);
			nombre_sleep_recibido = malloc(MAX_LENGTH);

			if(!recv_io_gen_sleep(fd_kernel, pcb_sleep, &unidades_de_trabajo, nombre_sleep_recibido)) {
				log_error(entradasalida_logger, "Hubo un error al recibir la interfaz IO_GEN_SLEEP");
			}
			strcpy(nombre_sleep, nombre_sleep_recibido);

			uint32_t usleep_final = TIEMPO_UNIDAD_TRABAJO * unidades_de_trabajo;
			
			log_info(entradasalida_logger, "PID: %d - Operacion: %s", pcb_sleep->pid, "IO_GEN_SLEEP");
			// Hace el sleep
			usleep(usleep_final * 1000);
			log_info(entradasalida_logger, "Finalizo el usleep de %d - PID: %d", usleep_final, pcb_sleep->pid);

			// Avisa que ya no esta mas interrumpido el proceso
			send_fin_io(fd_kernel, pcb_sleep->pid, nombre_sleep, strlen(nombre_sleep) + 1);

			// Liberar memoria
			free(pcb_sleep->registros);
			free(pcb_sleep);
			free(nombre_sleep);
			free(nombre_sleep_recibido);
			pthread_mutex_unlock(&mutexIO);
			break;
		case IO_STDIN_READ:
			pthread_mutex_lock(&mutexIO);
			pcb_stdin = malloc(sizeof(t_pcb));
			pcb_stdin->registros = malloc(sizeof(t_registros));
			nombre_stdin = malloc(MAX_LENGTH);
			nombre_stdin_recibido = malloc(MAX_LENGTH);
			uint32_t direccion_fisica_stdin;
			uint32_t tamanio_maximo_stdin;

			if(!recv_io_stdin_stdout(fd_kernel, pcb_stdin, &direccion_fisica_stdin, &tamanio_maximo_stdin, nombre_stdin_recibido)) {
				log_error(entradasalida_logger, "Hubo un error al recibir la interfaz IO_STDIN_READ");
			}
			strcpy(nombre_stdin, nombre_stdin_recibido);

			log_info(entradasalida_logger, "PID: %d - Operacion: %s", pcb_stdin->pid, "IO_STDIN_READ");
			log_info(entradasalida_logger, "Ingrese el string (maximo %d letras): ", tamanio_maximo_stdin);
			
			// Pedir string
			string = readline("> ");
			if (strlen(string) > tamanio_maximo_stdin) {
				log_info(entradasalida_logger, "El string supera los valores maximos.");
				send_fin_io(fd_kernel, pcb_stdin->pid, nombre_stdin, strlen(nombre_stdin) + 1);
				free(pcb_stdin->registros);
				free(pcb_stdin);
				free(nombre_stdin);
				free(nombre_stdin_recibido);
				break;
			}

			// Escribir en memoria (recibir escritura_ok)
			send_escribir_memoria(fd_memoria, pcb_stdin->pid, direccion_fisica_stdin, string, strlen(string) + 1);
			break;
		case IO_STDOUT_WRITE:
			pthread_mutex_lock(&mutexIO);
			pcb_stdout = malloc(sizeof(t_pcb));
			pcb_stdout->registros = malloc(sizeof(t_registros));
			nombre_stdout = malloc(MAX_LENGTH);
			nombre_stdout_recibido = malloc(MAX_LENGTH);
			uint32_t direccion_fisica_stdout;
			uint32_t tamanio_maximo_stdout;

			if(!recv_io_stdin_stdout(fd_kernel, pcb_stdout, &direccion_fisica_stdout, &tamanio_maximo_stdout, nombre_stdout_recibido)) {
				log_error(entradasalida_logger, "Hubo un error al recibir la interfaz IO_STDOUT_WRITE");
			}
			strcpy(nombre_stdout, nombre_stdout_recibido);

			log_info(entradasalida_logger, "PID: %d - Operacion: %s", pcb_stdout->pid, "IO_STDOUT_WRITE");

			// Logica de leer memoria y mostrar en pantalla el string (recibir_valor)
			send_leer_memoria(fd_memoria, pcb_stdout->pid, direccion_fisica_stdout, tamanio_maximo_stdout);
			break;
		case IO_FS_CREATE:
			pthread_mutex_lock(&mutexIO);
			t_pcb* pcb_fs_create = malloc(sizeof(t_pcb));
			pcb_fs_create->registros = malloc(sizeof(t_registros));
			nombre_fs = malloc(MAX_LENGTH);
			nombre_fs_recibido = malloc(MAX_LENGTH);
			nombre_archivo = malloc(MAX_LENGTH);
			nombre_archivo_recibido = malloc(MAX_LENGTH);

			if(!recv_io_fs_create_delete(fd_kernel, pcb_fs_create, nombre_archivo_recibido, nombre_fs_recibido)) {
				log_error(entradasalida_logger, "Hubo un error al recibir la interfaz IO_FS_CREATE");
			}
			strcpy(nombre_archivo, nombre_archivo_recibido);
			strcpy(nombre_fs, nombre_fs_recibido);

			usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
			
			log_info(entradasalida_logger, "PID: %d - Operacion: %s", pcb_fs_create->pid, "IO_FS_CREATE");
			log_info(entradasalida_logger, "PID: %d - Crear Archivo: %s", pcb_fs_create->pid, nombre_archivo);

			// Logica de cada instruccion
			crear_archivo(nombre_archivo, bitmap_blocks);

			// Avisa que ya no esta mas interrumpido el proceso
			send_fin_io(fd_kernel, pcb_fs_create->pid, nombre_fs, strlen(nombre_fs) + 1);

			// Liberar memoria
			free(pcb_fs_create->registros);
			free(pcb_fs_create);
			free(nombre_fs);
			free(nombre_fs_recibido);
			free(nombre_archivo);
			free(nombre_archivo_recibido);
			pthread_mutex_unlock(&mutexIO);
			break;
		case IO_FS_DELETE:
			pthread_mutex_lock(&mutexIO);
			t_pcb* pcb_fs_delete = malloc(sizeof(t_pcb));
			pcb_fs_delete->registros = malloc(sizeof(t_registros));
			nombre_fs = malloc(MAX_LENGTH);
			nombre_fs_recibido = malloc(MAX_LENGTH);
			nombre_archivo = malloc(MAX_LENGTH);
			nombre_archivo_recibido = malloc(MAX_LENGTH);

			if(!recv_io_fs_create_delete(fd_kernel, pcb_fs_delete, nombre_archivo_recibido, nombre_fs_recibido)) {
				log_error(entradasalida_logger, "Hubo un error al recibir la interfaz IO_FS_DELETE");
			}
			strcpy(nombre_archivo, nombre_archivo_recibido);
			strcpy(nombre_fs, nombre_fs_recibido);

			usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
			
			log_info(entradasalida_logger, "PID: %d - Operacion: %s", pcb_fs_delete->pid, "IO_FS_DELETE");
			log_info(entradasalida_logger, "PID: %d - Eliminar Archivo: %s", pcb_fs_delete->pid, nombre_archivo);

			// Logica de cada instruccion
			borrar_archivo(nombre_archivo, bitmap_blocks);

			// Avisa que ya no esta mas interrumpido el proceso
			send_fin_io(fd_kernel, pcb_fs_delete->pid, nombre_fs, strlen(nombre_fs) + 1);

			// Liberar memoria
			free(pcb_fs_delete->registros);
			free(pcb_fs_delete);
			free(nombre_fs);
			free(nombre_fs_recibido);
			free(nombre_archivo);
			free(nombre_archivo_recibido);
			pthread_mutex_unlock(&mutexIO);
			break;
		case IO_FS_TRUNCATE:
			pthread_mutex_lock(&mutexIO);
			t_pcb* pcb_fs_truncate = malloc(sizeof(t_pcb));
			pcb_fs_truncate->registros = malloc(sizeof(t_registros));
			uint32_t tamanio_truncate;
			nombre_fs = malloc(MAX_LENGTH);
			nombre_fs_recibido = malloc(MAX_LENGTH);
			nombre_archivo = malloc(MAX_LENGTH);
			nombre_archivo_recibido = malloc(MAX_LENGTH);

			if(!recv_io_fs_truncate(fd_kernel, pcb_fs_truncate, &tamanio_truncate, nombre_archivo_recibido, nombre_fs_recibido)) {
				log_error(entradasalida_logger, "Hubo un error al recibir la interfaz IO_FS_TRUNCATE");
			}
			strcpy(nombre_archivo, nombre_archivo_recibido);
			strcpy(nombre_fs, nombre_fs_recibido);

			usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
			
			log_info(entradasalida_logger, "PID: %d - Operacion: %s", pcb_fs_truncate->pid, "IO_FS_TRUNCATE");
			log_info(entradasalida_logger, "PID: %d - Truncar Archivo: %s - Tamaño: %d", pcb_fs_truncate->pid, nombre_archivo, tamanio_truncate);

			// Logica de cada instruccion
			truncate_archivo(nombre_archivo, tamanio_truncate, bitmap_blocks, pcb_fs_truncate);

			// Avisa que ya no esta mas interrumpido el proceso
			send_fin_io(fd_kernel, pcb_fs_truncate->pid, nombre_fs, strlen(nombre_fs) + 1);

			// Liberar memoria
			free(pcb_fs_truncate->registros);
			free(pcb_fs_truncate);
			free(nombre_fs);
			free(nombre_fs_recibido);
			free(nombre_archivo);
			free(nombre_archivo_recibido);
			pthread_mutex_unlock(&mutexIO);
			break;
		case IO_FS_WRITE:
			pthread_mutex_lock(&mutexIO);
			pcb_fs_write = malloc(sizeof(t_pcb));
			pcb_fs_write->registros = malloc(sizeof(t_registros));
			uint32_t tamanio_write, dir_fisica_write;
			nombre_fs = malloc(MAX_LENGTH);
			nombre_fs_recibido = malloc(MAX_LENGTH);
			nombre_archivo = malloc(MAX_LENGTH);
			nombre_archivo_recibido = malloc(MAX_LENGTH);

			if(!recv_io_fs_write_read(fd_kernel, pcb_fs_write, &tamanio_write, &dir_fisica_write, &ptr_archivo_write, nombre_archivo_recibido, nombre_fs_recibido)) {
				log_error(entradasalida_logger, "Hubo un error al recibir la interfaz IO_FS_WRITE");
			}
			strcpy(nombre_archivo, nombre_archivo_recibido);
			strcpy(nombre_fs, nombre_fs_recibido);

			usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
			
			log_info(entradasalida_logger, "PID: %d - Operacion: %s", pcb_fs_write->pid, "IO_FS_WRITE");
			log_info(entradasalida_logger, "PID: %d - Escribir Archivo: %s - Tamaño a Escribir: %d - Puntero Archivo: %d", pcb_fs_write->pid, nombre_archivo, tamanio_write, ptr_archivo_write);

			// Logica de cada instruccion
			send_leer_memoria(fd_memoria, pcb_fs_write->pid, dir_fisica_write, tamanio_write);
			break;
		case IO_FS_READ:
			pthread_mutex_lock(&mutexIO);
			pcb_fs_read = malloc(sizeof(t_pcb));
			pcb_fs_read->registros = malloc(sizeof(t_registros));
			uint32_t tamanio_read, dir_fisica_read, ptr_archivo_read;
			nombre_fs = malloc(MAX_LENGTH);
			nombre_fs_recibido = malloc(MAX_LENGTH);
			nombre_archivo = malloc(MAX_LENGTH);
			nombre_archivo_recibido = malloc(MAX_LENGTH);

			if(!recv_io_fs_write_read(fd_kernel, pcb_fs_read, &tamanio_read, &dir_fisica_read, &ptr_archivo_read, nombre_archivo_recibido, nombre_fs_recibido)) {
				log_error(entradasalida_logger, "Hubo un error al recibir la interfaz IO_FS_READ");
			}
			strcpy(nombre_archivo, nombre_archivo_recibido);
			strcpy(nombre_fs, nombre_fs_recibido);

			usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
			
			log_info(entradasalida_logger, "PID: %d - Operacion: %s", pcb_fs_read->pid, "IO_FS_READ");
			log_info(entradasalida_logger, "PID: %d - Leer Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d", pcb_fs_read->pid, nombre_archivo, tamanio_read, ptr_archivo_read);

			// Logica de cada instruccion
			read_archivo(nombre_archivo, tamanio_read, dir_fisica_read, ptr_archivo_read, pcb_fs_read, bitmap_blocks);
			break;
		case -1:
			log_error(entradasalida_logger, "El servidor de Kernel no se encuentra activo.");
			control = 0;
            break;
		default:
			log_warning(entradasalida_logger, "Operacion desconocida.");
			break;
		}
	}
}