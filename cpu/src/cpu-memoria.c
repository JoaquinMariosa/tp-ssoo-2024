#include "../include/cpu-memoria.h"

t_dictionary* dictionary_registros;
char* instruccion;
char* instruccion_recibida;
char** instruccion_separada;

uint32_t tam_pagina;
t_list* lista_tlb;
pthread_mutex_t pcbEjecutarMutex = PTHREAD_MUTEX_INITIALIZER;
bool esperando_datos;

void printear_pcb() {
	log_info(cpu_logger, "PID: %d\nProgram Counter: %d\nEstado: %c\nQuantum: %d\nFlag IO: %d\nRegistros:\nAX: %d, BX: %d, CX: %d, DX: %d\nEAX: %d, EBX: %d, ECX: %d, EDX: %d\nSI: %d, DI: %d",
		pcb_a_ejecutar->pid, pcb_a_ejecutar->pc, pcb_a_ejecutar->estado, pcb_a_ejecutar->quantum, pcb_a_ejecutar->flag_int,
		pcb_a_ejecutar->registros->AX, pcb_a_ejecutar->registros->BX, pcb_a_ejecutar->registros->CX, pcb_a_ejecutar->registros->DX,
		pcb_a_ejecutar->registros->EAX, pcb_a_ejecutar->registros->EBX, pcb_a_ejecutar->registros->ECX, pcb_a_ejecutar->registros->EDX,
		pcb_a_ejecutar->registros->SI, pcb_a_ejecutar->registros->DI);
}

void free_instruccion_pendiente(t_instruccion_pendiente* instruccion_pendiente) {
	// Liberar la memoria de la instruccion pendiente
    free(instruccion_pendiente->registro_datos);
    free(instruccion_pendiente->registro_direccion);

	if (strcmp(instruccion_pendiente->instruccion, "COPY_STRING") == 0) {
		free(instruccion_pendiente->datos);
	}

    if (strcmp(instruccion_pendiente->instruccion, "IO_STDIN_READ") == 0 || 
        strcmp(instruccion_pendiente->instruccion, "IO_STDOUT_WRITE") == 0 || 
        strcmp(instruccion_pendiente->instruccion, "IO_FS_READ") == 0 || 
        strcmp(instruccion_pendiente->instruccion, "IO_FS_WRITE") == 0) {
        free(instruccion_pendiente->nombre_interfaz);
    }

    if (strcmp(instruccion_pendiente->instruccion, "IO_FS_READ") == 0 || 
        strcmp(instruccion_pendiente->instruccion, "IO_FS_WRITE") == 0) {
        free(instruccion_pendiente->nombre_archivo);
    }

	free(instruccion_pendiente->instruccion);
	free(instruccion_pendiente);
}

void crear_diccionario(t_dictionary* dictionary_registros) {
    dictionary_put(dictionary_registros, "AX", &pcb_a_ejecutar->registros->AX);
    dictionary_put(dictionary_registros, "BX", &pcb_a_ejecutar->registros->BX);
    dictionary_put(dictionary_registros, "CX", &pcb_a_ejecutar->registros->CX);
    dictionary_put(dictionary_registros, "DX", &pcb_a_ejecutar->registros->DX);
    dictionary_put(dictionary_registros, "EAX", &pcb_a_ejecutar->registros->EAX);
    dictionary_put(dictionary_registros, "EBX", &pcb_a_ejecutar->registros->EBX);
    dictionary_put(dictionary_registros, "ECX", &pcb_a_ejecutar->registros->ECX);
    dictionary_put(dictionary_registros, "EDX", &pcb_a_ejecutar->registros->EDX);
    dictionary_put(dictionary_registros, "SI", &pcb_a_ejecutar->registros->SI);
    dictionary_put(dictionary_registros, "DI", &pcb_a_ejecutar->registros->DI);
    dictionary_put(dictionary_registros, "PC", &pcb_a_ejecutar->pc);
}

void conexion_cpu_memoria() {
    bool control = 1;
	lista_tlb = list_create();
	esperando_datos = false;
	while (control) {
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op) {
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case RECIBIR_TAM_PAGINA:
			if (!recv_tam_pagina(fd_memoria, &tam_pagina)) {
				log_error(cpu_logger, "Hubo un problema al recibir el TAM_PAGINA");
				break;
			}
			break;
		case RECIBIR_INSTRUCCION:
			int MAX_LENGTH = 128;
			instruccion = malloc(MAX_LENGTH);
			instruccion_recibida = malloc(MAX_LENGTH);
			if(pcb_a_ejecutar != NULL) {
				// Si habia una instruccion pendiente, que la libere
				if(instruccion_pendiente != NULL) {
					free_instruccion_pendiente(instruccion_pendiente);
					instruccion_pendiente = NULL;
				}
				// Creo diccionario
				dictionary_registros = dictionary_create();
				crear_diccionario(dictionary_registros);

				// Recibir la instruccion
				if (!recv_instruccion(fd_memoria, instruccion_recibida)) {
					log_error(cpu_logger, "Hubo un error al recibir la instruccion del modulo de Memoria");
					free(instruccion);
					free(instruccion_recibida);
					dictionary_destroy(dictionary_registros);
					break;
				}

				strcpy(instruccion, instruccion_recibida);

				// Decode
				pthread_mutex_lock(&pcbEjecutarMutex);
				instruccion_separada = string_split(instruccion, " ");
				if (strcmp(instruccion_separada[0], "SET") == 0) {
					int valor = atoi(instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %d", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], valor);
					funcion_set(dictionary_registros, instruccion_separada[1], valor);
				} else if (strcmp(instruccion_separada[0], "SUM") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
					funcion_sum(dictionary_registros, instruccion_separada[1], instruccion_separada[2]);
				} else if (strcmp(instruccion_separada[0], "SUB") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
					funcion_sub(dictionary_registros, instruccion_separada[1], instruccion_separada[2]);
				} else if (strcmp(instruccion_separada[0], "MOV_IN") == 0) {
					string_trim_right(&instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
					funcion_mov_in(dictionary_registros, instruccion_separada[1], instruccion_separada[2]);
					// Tiene que esperar a recibir un dato
					esperando_datos = true;
				} else if (strcmp(instruccion_separada[0], "MOV_OUT") == 0) {
					string_trim_right(&instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
					funcion_mov_out(dictionary_registros, instruccion_separada[1], instruccion_separada[2]);
					// Tiene que esperar a recibir un dato
					esperando_datos = true;
				} else if (strcmp(instruccion_separada[0], "JNZ") == 0) {
					uint32_t valor_pc = atoi(instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %d", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], valor_pc);
					funcion_jnz(dictionary_registros, instruccion_separada[1], valor_pc);
				} else if (strcmp(instruccion_separada[0], "RESIZE") == 0) {
					uint32_t tamanio = atoi(instruccion_separada[1]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %d", pcb_a_ejecutar->pid, instruccion_separada[0], tamanio);
					funcion_resize(tamanio);
					// Tiene que esperar a recibir un dato
					esperando_datos = true;
				} else if (strcmp(instruccion_separada[0], "COPY_STRING") == 0) {
					uint32_t tamanio = atoi(instruccion_separada[1]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %d", pcb_a_ejecutar->pid, instruccion_separada[0], tamanio);
					funcion_copy_string(dictionary_registros, tamanio);
					// Tiene que esperar a recibir un dato
					esperando_datos = true;
				}  else if (strcmp(instruccion_separada[0], "WAIT") == 0) {
					string_trim_right(&instruccion_separada[1]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1]);
					funcion_wait(instruccion_separada[1]);
					// Tiene que esperar a recibir un dato
					esperando_datos = true;
				}  else if (strcmp(instruccion_separada[0], "SIGNAL") == 0) {
					string_trim_right(&instruccion_separada[1]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1]);
					funcion_signal(instruccion_separada[1]);
					// Tiene que esperar a recibir un dato
					esperando_datos = true;
				} else if (strcmp(instruccion_separada[0], "IO_GEN_SLEEP") == 0) {
					uint32_t unidades_trabajo = atoi(instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %d", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], unidades_trabajo);
					funcion_io_gen_sleep(instruccion_separada[1], unidades_trabajo);
				} else if (strcmp(instruccion_separada[0], "IO_STDIN_READ") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[3]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2], instruccion_separada[3]);
					funcion_io_stdin_read(dictionary_registros, instruccion_separada[1], instruccion_separada[2], instruccion_separada[3]);
					// Si es TLB miss se queda esperando datos sino solo interrumpe el proceso por IO
				} else if (strcmp(instruccion_separada[0], "IO_STDOUT_WRITE") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[3]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2], instruccion_separada[3]);
					funcion_io_stdout_write(dictionary_registros, instruccion_separada[1], instruccion_separada[2], instruccion_separada[3]);
					// Si es TLB miss se queda esperando datos sino solo interrumpe el proceso por IO
				} else if (strcmp(instruccion_separada[0], "IO_FS_CREATE") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
					funcion_io_fs_create(instruccion_separada[1], instruccion_separada[2]);
				} else if (strcmp(instruccion_separada[0], "IO_FS_DELETE") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[2]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
					funcion_io_fs_delete(instruccion_separada[1], instruccion_separada[2]);
				} else if (strcmp(instruccion_separada[0], "IO_FS_TRUNCATE") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[3]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2], instruccion_separada[3]);
					funcion_io_fs_truncate(dictionary_registros, instruccion_separada[1], instruccion_separada[2], instruccion_separada[3]);
				} else if (strcmp(instruccion_separada[0], "IO_FS_WRITE") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[5]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s %s %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2], instruccion_separada[3], instruccion_separada[4], instruccion_separada[5]);
					funcion_io_fs_write(dictionary_registros, instruccion_separada[1], instruccion_separada[2], instruccion_separada[3], instruccion_separada[4], instruccion_separada[5]);
					// Si es TLB miss se queda esperando datos sino solo interrumpe el proceso por IO
				} else if (strcmp(instruccion_separada[0], "IO_FS_READ") == 0) {
					// Elimino \n cuando es un string la ultima parte de la instruccion
					string_trim_right(&instruccion_separada[5]);
					log_info(cpu_logger, "PID: %d - Ejecutando: %s - %s %s %s %s %s", pcb_a_ejecutar->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2], instruccion_separada[3], instruccion_separada[4], instruccion_separada[5]);
					funcion_io_fs_read(dictionary_registros, instruccion_separada[1], instruccion_separada[2], instruccion_separada[3], instruccion_separada[4], instruccion_separada[5]);
					// Si es TLB miss se queda esperando datos sino solo interrumpe el proceso por IO
				} else if (strcmp(instruccion_separada[0], "EXIT") == 0) {
					log_info(cpu_logger, "PID: %d - Ejecutando: %s", pcb_a_ejecutar->pid, instruccion_separada[0]);
					funcion_exit();
				} else {
					log_warning(cpu_logger, "No se pudo encontrar la instruccion: %s", instruccion_separada[0]);
				}
				pthread_mutex_unlock(&pcbEjecutarMutex);

				// Espera a recibir el dato para las instrucciones necesarias
				if (esperando_datos) {
					break;
				}

				// Ver si es EXIT
				if(pcb_a_ejecutar == NULL) {
					log_info(cpu_logger, "El proceso finalizo");
					free(instruccion);
					free(instruccion_recibida);
					for (int i = 0; instruccion_separada[i] != NULL; i++) {
						free(instruccion_separada[i]);
					}
					free(instruccion_separada);
					dictionary_destroy(dictionary_registros);
					break;
				}

				log_info(cpu_logger, "Instruccion finalizada");

				// Printea el PCB
				// printear_pcb(pcb_a_ejecutar);
				
				// Interrupcion por IO
				if(pcb_a_ejecutar->flag_int == 1) {
					log_info(cpu_logger, "Proceso %d fue interrumpido por una IO", pcb_a_ejecutar->pid);
					free(instruccion);
					free(instruccion_recibida);
					for (int i = 0; instruccion_separada[i] != NULL; i++) {
						free(instruccion_separada[i]);
					}
					free(instruccion_separada);
					dictionary_destroy(dictionary_registros);
					break;
				}

				// Interrupcion por fin de Quantum
				if(pcb_a_ejecutar->flag_int == 2) {
					log_info(cpu_logger, "Proceso %d finalizo su quantum", pcb_a_ejecutar->pid);
					send_pcb(fd_kernel_dispatch, pcb_a_ejecutar);
					free(instruccion);
					free(instruccion_recibida);
					for (int i = 0; instruccion_separada[i] != NULL; i++) {
						free(instruccion_separada[i]);
					}
					free(instruccion_separada);
					dictionary_destroy(dictionary_registros);
					break;
				}

				// Interrupcion por fin FINALIZAR_PROCESO
				if(pcb_a_ejecutar->flag_int == 3) {
					log_info(cpu_logger, "Proceso %d fue interrumpido por el usuario (FINALIZAR_PROCESO)", pcb_a_ejecutar->pid);
					send_pcb_fp(fd_kernel_dispatch, pcb_a_ejecutar);
					free(pcb_a_ejecutar->registros);
					free(pcb_a_ejecutar);
					pcb_a_ejecutar = NULL;
					free(instruccion);
					free(instruccion_recibida);
					for (int i = 0; instruccion_separada[i] != NULL; i++) {
						free(instruccion_separada[i]);
					}
					free(instruccion_separada);
					dictionary_destroy(dictionary_registros);
					break;
				}

				// Fetch --> seguir pidiendo instrucciones
				send_pc_pid(fd_memoria, pcb_a_ejecutar->pc, pcb_a_ejecutar->pid);
				log_info(cpu_logger, "PID: %d - FETCH - Program Counter: %d", pcb_a_ejecutar->pid, pcb_a_ejecutar->pc);

				// Libera la instruccion anterior
				free(instruccion);
				free(instruccion_recibida);
				// Liberar el arreglo de la instrucción separada
				for (int i = 0; instruccion_separada[i] != NULL; i++) {
					free(instruccion_separada[i]);
				}
				free(instruccion_separada);
				dictionary_destroy(dictionary_registros);
			}
			break;
        case RECIBIR_MARCO: // TLB Miss
			uint8_t pid_marco;
			uint32_t numero_pagina, marco;
			if (!recv_num_marco(fd_memoria, &pid_marco, &numero_pagina, &marco)) {
				log_error(cpu_logger, "Error al recibir el marco de la memoria");
				break;
			}

			log_info(cpu_logger, "PID: %d - OBTENER MARCO - Pagina: %d - Marco: %d", pid_marco, numero_pagina, marco);
			
			if (CANTIDAD_ENTRADAS_TLB != 0) {
				agregar_a_tlb(pid_marco, numero_pagina, marco);
			}

			// Calculo para la instruccion que esperaba recibir algun dato, como un marco, etc.
			uint32_t desplazamiento = instruccion_pendiente->direccion_logica - numero_pagina * tam_pagina;
			uint32_t direccion_fisica = marco * tam_pagina + desplazamiento;

			// Continuar la operación pendiente si existe
			pthread_mutex_lock(&instruccion_pendiente_mutex);
			if (instruccion_pendiente != NULL) {
				// Realizar la operación pendiente
				if (strcmp(instruccion_pendiente->instruccion, "MOV_IN") == 0) {
					if (strlen(instruccion_pendiente->registro_datos) == 3 || !strcmp(instruccion_pendiente->registro_datos, "SI") || !strcmp(instruccion_pendiente->registro_datos, "DI") || !strcmp(instruccion_pendiente->registro_datos, "PC")) {
						uint32_t tamanio_a_leer = sizeof(uint32_t);
						send_leer_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, tamanio_a_leer);
					} else if (strlen(instruccion_pendiente->registro_datos) == 2) {
						uint32_t tamanio_a_leer = sizeof(uint8_t);
						send_leer_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, tamanio_a_leer);
					}

    				pcb_a_ejecutar->pc++;
				} else if (strcmp(instruccion_pendiente->instruccion, "MOV_OUT") == 0) {
					if (strlen(instruccion_pendiente->registro_datos) == 3 || !strcmp(instruccion_pendiente->registro_datos, "SI") || !strcmp(instruccion_pendiente->registro_datos, "DI") || !strcmp(instruccion_pendiente->registro_datos, "PC")) {
						uint32_t *reg_datos = dictionary_get(dictionary_registros, instruccion_pendiente->registro_datos);
						uint32_t tamanio_a_escribir = sizeof(uint32_t);
        				log_info(cpu_logger, "PID: %d - Accion: ESCRIBIR - Direccion Fisica: %d - Valor: %d", pcb_a_ejecutar->pid, direccion_fisica, *reg_datos);
						send_escribir_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, reg_datos, tamanio_a_escribir);
					} else if (strlen(instruccion_pendiente->registro_datos) == 2) {
						uint8_t *reg_datos = dictionary_get(dictionary_registros, instruccion_pendiente->registro_datos);
						uint32_t tamanio_a_escribir = sizeof(uint8_t);
        				log_info(cpu_logger, "PID: %d - Accion: ESCRIBIR - Direccion Fisica: %d - Valor: %d", pcb_a_ejecutar->pid, direccion_fisica, *reg_datos);
						send_escribir_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, reg_datos, tamanio_a_escribir);
					}
				} else if (strcmp(instruccion_pendiente->instruccion, "COPY_STRING") == 0) {
					if (strcmp(instruccion_pendiente->registro_datos, "SI") == 0) {
                        send_leer_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, instruccion_pendiente->tamanio);
                    } else if (strcmp(instruccion_pendiente->registro_datos, "DI") == 0) {
						log_info(cpu_logger, "PID: %d - Accion: ESCRIBIR - Direccion Fisica: %d - Valor: %.*s", pcb_a_ejecutar->pid, direccion_fisica, instruccion_pendiente->tamanio, (char*)instruccion_pendiente->datos);
                        send_escribir_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, instruccion_pendiente->datos, instruccion_pendiente->tamanio);
                    }
				} else if (strcmp(instruccion_pendiente->instruccion, "IO_STDIN_READ") == 0) {
    				send_io_stdin_read(fd_kernel_dispatch, pcb_a_ejecutar, direccion_fisica, instruccion_pendiente->tamanio, instruccion_pendiente->nombre_interfaz, strlen(instruccion_pendiente->nombre_interfaz) + 1);
					esperando_datos = false;
					// free_instruccion_pendiente(instruccion_pendiente);
				} else if (strcmp(instruccion_pendiente->instruccion, "IO_STDOUT_WRITE") == 0) {
    				send_io_stdout_write(fd_kernel_dispatch, pcb_a_ejecutar, direccion_fisica, instruccion_pendiente->tamanio, instruccion_pendiente->nombre_interfaz, strlen(instruccion_pendiente->nombre_interfaz) + 1);
					esperando_datos = false;
					// free_instruccion_pendiente(instruccion_pendiente);
				} else if (strcmp(instruccion_pendiente->instruccion, "IO_FS_WRITE") == 0) {
    				send_io_fs_write(fd_kernel_dispatch, pcb_a_ejecutar, instruccion_pendiente->tamanio, direccion_fisica, instruccion_pendiente->puntero_archivo, instruccion_pendiente->nombre_archivo, strlen(instruccion_pendiente->nombre_archivo) + 1, instruccion_pendiente->nombre_interfaz, strlen(instruccion_pendiente->nombre_interfaz) + 1);
					esperando_datos = false;
					// free_instruccion_pendiente(instruccion_pendiente);
				} else if (strcmp(instruccion_pendiente->instruccion, "IO_FS_READ") == 0) {
    				send_io_fs_read(fd_kernel_dispatch, pcb_a_ejecutar, instruccion_pendiente->tamanio, direccion_fisica, instruccion_pendiente->puntero_archivo, instruccion_pendiente->nombre_archivo, strlen(instruccion_pendiente->nombre_archivo) + 1, instruccion_pendiente->nombre_interfaz, strlen(instruccion_pendiente->nombre_interfaz) + 1);
					esperando_datos = false;
					// free_instruccion_pendiente(instruccion_pendiente);
				}
			}
			pthread_mutex_unlock(&instruccion_pendiente_mutex);

			// Interrupcion por IO
			if(pcb_a_ejecutar->flag_int == 1) {
				log_info(cpu_logger, "Proceso %d fue interrumpido por una IO", pcb_a_ejecutar->pid);
				free(instruccion);
				free(instruccion_recibida);
				for (int i = 0; instruccion_separada[i] != NULL; i++) {
					free(instruccion_separada[i]);
				}
				free(instruccion_separada);
				dictionary_destroy(dictionary_registros);
				break;
			}
			
			break;
		case RECIBIR_VALOR_MEMORIA: // Para MOV_IN y COPY_STRING
			// Recibir el valor de memoria y continuar la operación pendiente si existente
			void* valor;
    		uint32_t tam_dato;
			uint32_t direccion_fisica_leer;
			if (!recv_valor_memoria(fd_memoria, &direccion_fisica_leer, &valor, &tam_dato)) {
				log_error(cpu_logger, "Hubo un error al recibir el valor de memoria");
				break;
			}

			// Continuar la operación pendiente si existe
			pthread_mutex_lock(&instruccion_pendiente_mutex);
			if (instruccion_pendiente != NULL) {
				// Realizar la operación pendiente
				if (strcmp(instruccion_pendiente->instruccion, "MOV_IN") == 0) {
					// Asignar el valor recibido al registro de datos dependiendo del tipo de dato
					if (tam_dato == sizeof(uint32_t)) {
						uint32_t *reg_datos = dictionary_get(dictionary_registros, instruccion_pendiente->registro_datos);
						*reg_datos = *((uint32_t*)valor);
						log_info(cpu_logger, "PID: %d - Accion: LEER - Direccion Fisica: %d - Valor: %d", pcb_a_ejecutar->pid, direccion_fisica_leer, *reg_datos);
					} else if (tam_dato == sizeof(uint8_t)) {
						uint8_t *reg_datos = dictionary_get(dictionary_registros, instruccion_pendiente->registro_datos);
						*reg_datos = *((uint8_t*)valor);
						log_info(cpu_logger, "PID: %d - Accion: LEER - Direccion Fisica: %d - Valor: %d", pcb_a_ejecutar->pid, direccion_fisica_leer, *reg_datos);
					} else {
						log_error(cpu_logger, "Tamaño de dato desconocido: %d", tam_dato);
					}
				} else if (strcmp(instruccion_pendiente->instruccion, "COPY_STRING") == 0) {
					free_instruccion_pendiente(instruccion_pendiente);
					uint32_t *reg_di = dictionary_get(dictionary_registros, "DI");
					int direccion_fisica_di = mmu(*reg_di);
					if (direccion_fisica_di == -1) {
						// TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
						instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
						instruccion_pendiente->instruccion = strdup("COPY_STRING");
						instruccion_pendiente->direccion_logica = *reg_di;
						instruccion_pendiente->registro_datos = strdup("DI");
						instruccion_pendiente->datos = valor;
						instruccion_pendiente->tamanio = tam_dato;
						instruccion_pendiente->registro_direccion = NULL; // No tiene
						instruccion_pendiente->nombre_interfaz = NULL; // No tiene
						instruccion_pendiente->puntero_archivo = 0; // No tiene
						instruccion_pendiente->nombre_archivo = NULL; // No tiene
						uint32_t numero_pagina = *reg_di / tam_pagina;
						uint32_t desplazamiento = *reg_di - numero_pagina * tam_pagina;
						send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
						pthread_mutex_unlock(&instruccion_pendiente_mutex);
						break;
					}

					// TLB hit
					log_info(cpu_logger, "PID: %d - Accion: ESCRIBIR - Direccion Fisica: %d - Valor: %.*s", pcb_a_ejecutar->pid, direccion_fisica_di, tam_dato, (char*)valor);
					send_escribir_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica_di, valor, tam_dato);
					break;
				}
				// Liberar la memoria de la instrucción pendiente
				// free_instruccion_pendiente(instruccion_pendiente);
			}
			pthread_mutex_unlock(&instruccion_pendiente_mutex);

			// Ya no se esperan datos
			esperando_datos = false;
			
			log_info(cpu_logger, "Instruccion finalizada");
			// Printea el PCB
			// printear_pcb(pcb_a_ejecutar);

			// Interrupcion por fin de Quantum
			if(pcb_a_ejecutar->flag_int == 2) {
				log_info(cpu_logger, "Proceso %d finalizo su quantum", pcb_a_ejecutar->pid);
				send_pcb(fd_kernel_dispatch, pcb_a_ejecutar);
				free(instruccion);
				free(instruccion_recibida);
				for (int i = 0; instruccion_separada[i] != NULL; i++) {
					free(instruccion_separada[i]);
				}
				free(instruccion_separada);
				// free_instruccion_pendiente(instruccion_pendiente);
				dictionary_destroy(dictionary_registros);
				break;
			}

			// Interrupcion por fin FINALIZAR_PROCESO
			if(pcb_a_ejecutar->flag_int == 3) {
				log_info(cpu_logger, "Proceso %d fue interrumpido por el usuario (FINALIZAR_PROCESO)", pcb_a_ejecutar->pid);
				send_pcb_fp(fd_kernel_dispatch, pcb_a_ejecutar);
				free(pcb_a_ejecutar->registros);
				free(pcb_a_ejecutar);
				pcb_a_ejecutar = NULL;
				free(instruccion);
				free(instruccion_recibida);
				for (int i = 0; instruccion_separada[i] != NULL; i++) {
					free(instruccion_separada[i]);
				}
				free(instruccion_separada);
				dictionary_destroy(dictionary_registros);
				break;
			}

			// Libera la instruccion anterior
			free(instruccion);
			free(instruccion_recibida);
			// Liberar el arreglo de la instrucción separada
			for (int i = 0; instruccion_separada[i] != NULL; i++) {
				free(instruccion_separada[i]);
			}
			free(instruccion_separada);
			// free_instruccion_pendiente(instruccion_pendiente);
			dictionary_destroy(dictionary_registros);

			// Fetch --> seguir pidiendo instrucciones
			send_pc_pid(fd_memoria, pcb_a_ejecutar->pc, pcb_a_ejecutar->pid);
			log_info(cpu_logger, "PID: %d - FETCH - Program Counter: %d", pcb_a_ejecutar->pid, pcb_a_ejecutar->pc);
			break;
		case ESCRITURA_OK: // Para MOV_OUT y COPY_STRING
			uint8_t escritura_ok;
			if(!recv_escritura_ok(fd_memoria, &escritura_ok)) {
				log_error(cpu_logger, "Hubo un error al recibir el OK de la escritura");
			}

			if(escritura_ok == 1) {
				log_info(cpu_logger, "Escritura OK");
			} else {
				log_error(cpu_logger, "Hubo un error en la escritura");
			}

			// Ya no se esperan datos
			pcb_a_ejecutar->pc++;
			esperando_datos = false;

			log_info(cpu_logger, "Instruccion finalizada");
			// Printea el PCB
			// printear_pcb(pcb_a_ejecutar);

			// Interrupcion por fin de Quantum
			if(pcb_a_ejecutar->flag_int == 2) {
				log_info(cpu_logger, "Proceso %d finalizo su quantum", pcb_a_ejecutar->pid);
				send_pcb(fd_kernel_dispatch, pcb_a_ejecutar);
				free(instruccion);
				free(instruccion_recibida);
				for (int i = 0; instruccion_separada[i] != NULL; i++) {
					free(instruccion_separada[i]);
				}
				free(instruccion_separada);
				// free_instruccion_pendiente(instruccion_pendiente);
				dictionary_destroy(dictionary_registros);
				break;
			}

			if(pcb_a_ejecutar->flag_int == 3) {
				log_info(cpu_logger, "Proceso %d fue interrumpido por el usuario (FINALIZAR_PROCESO)", pcb_a_ejecutar->pid);
				send_pcb_fp(fd_kernel_dispatch, pcb_a_ejecutar);
				free(pcb_a_ejecutar->registros);
				free(pcb_a_ejecutar);
				pcb_a_ejecutar = NULL;
				free(instruccion);
				free(instruccion_recibida);
				for (int i = 0; instruccion_separada[i] != NULL; i++) {
					free(instruccion_separada[i]);
				}
				free(instruccion_separada);
				dictionary_destroy(dictionary_registros);
				break;
			}

			// Libera la instruccion anterior
			free(instruccion);
			free(instruccion_recibida);
			// Liberar el arreglo de la instrucción separada
			for (int i = 0; instruccion_separada[i] != NULL; i++) {
				free(instruccion_separada[i]);
			}
			free(instruccion_separada);
			// free_instruccion_pendiente(instruccion_pendiente);
			dictionary_destroy(dictionary_registros);

			// Fetch --> seguir pidiendo instrucciones
			send_pc_pid(fd_memoria, pcb_a_ejecutar->pc, pcb_a_ejecutar->pid);
			log_info(cpu_logger, "PID: %d - FETCH - Program Counter: %d", pcb_a_ejecutar->pid, pcb_a_ejecutar->pc);
			break;
		case OUT_OF_MEMORY:
			uint8_t pid_oom;
			if(!recv_out_of_memory(fd_memoria, &pid_oom)) {
				log_error(cpu_logger, "Hubo un error al recibir el Out of Memory");
			}

			// Ya no se esperan datos
			esperando_datos = false;
			
			// Printea el PCB
			// printear_pcb(pcb_a_ejecutar);

			// Libera la instruccion anterior
			free(instruccion);
			free(instruccion_recibida);
			// Liberar el arreglo de la instrucción separada
			for (int i = 0; instruccion_separada[i] != NULL; i++) {
				free(instruccion_separada[i]);
			}
			free(instruccion_separada);
			dictionary_destroy(dictionary_registros);

			if(pid_oom != pcb_a_ejecutar->pid) {
				// Fetch --> seguir pidiendo instrucciones
				send_pc_pid(fd_memoria, pcb_a_ejecutar->pc, pcb_a_ejecutar->pid);
				log_info(cpu_logger, "PID: %d - FETCH - Program Counter: %d", pcb_a_ejecutar->pid, pcb_a_ejecutar->pc);
				break;
			} else {
				log_error(cpu_logger, "Fin del proceso por Out of Memory");
				break;
			}
			break;
		case -1:
			log_error(cpu_logger, "El servidor de Memoria no se encuentra activo.");
			control = 0;
            break;
		default:
			log_warning(cpu_logger, "Operacion desconocida.");
			break;
		}
	}
}