#include "../include/memoria-cpu.h"

pthread_mutex_t programCounter = PTHREAD_MUTEX_INITIALIZER;

void list_add_in_index_safe(t_list* list, int index, void* element) {
    // NULL si el indice es mayor que el tamanio actual
    while (list_size(list) < index) {
        list_add(list, NULL);
    }
    list_add_in_index(list, index, element);
}

void conexion_memoria_cpu() {
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op) {
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case RECIBIR_PC_PID:
            pthread_mutex_lock(&programCounter);
			uint8_t pid;
			uint32_t pc;
			if(!recv_pc_pid(fd_cpu, &pc, &pid)) { 
				log_error(memoria_logger, "Hubo un error al recibir el PC de un proceso");
			}

			usleep(RETARDO_RESPUESTA * 1000);

			t_list* instrucciones = list_get(instrucciones_por_proceso, pid);

			if (list_is_empty(instrucciones)) {
				log_warning(memoria_logger, "La lista de instrucciones se encuentra vacia");
                pthread_mutex_unlock(&programCounter);
                break;
            }
			
			if (pc <= list_size(instrucciones)) {
				char* instruccion = list_get(instrucciones, pc);
				send_instruccion(fd_cpu, instruccion, strlen(instruccion) + 1);
				log_info(memoria_logger, "Instruccion %d enviada", pc);
			}
            pthread_mutex_unlock(&programCounter);
			break;
		case RECIBIR_TAMANIO:
			uint8_t pid_resize;
			uint32_t tamanio;
			if(!recv_tamanio(fd_cpu, &tamanio, &pid_resize)) {
				log_error(memoria_logger, "Hubo un error al recibir el Resize");
			}

            usleep(RETARDO_RESPUESTA * 1000);

            int paginas_necesarias = ceil(tamanio / (float)TAM_PAGINA);

			t_list* tabla_paginas = list_get(tabla_paginas_por_proceso, pid_resize);
            int paginas_actuales = list_size(tabla_paginas);
            if (paginas_necesarias > paginas_actuales) {
                // Verificar si hay suficientes marcos disponibles antes de ampliar
                int marcos_disponibles = contar_marcos_libres(marcos_ocupados);
                int marcos_necesarios = paginas_necesarias - paginas_actuales;

                if (marcos_necesarios > marcos_disponibles) {
                    log_error(memoria_logger, "Out Of Memory: No hay suficientes marcos libres para ampliar el proceso");
                    send_out_of_memory(fd_kernel, pid_resize);
                    send_out_of_memory(fd_cpu, pid_resize);
                    break;
                }

                // Aumentar tamaño --> añadir mas marcos
                log_info(memoria_logger, "Ampliacion del Proceso");
                log_info(memoria_logger, "PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d", pid_resize, paginas_actuales * TAM_PAGINA, tamanio);
                for (int i = paginas_actuales; i < paginas_necesarias; i++) {
                    uint32_t marco = obtener_marco_libre(marcos_ocupados);
                    if (marco != -1) {
                        uint32_t* marco_ptr = malloc(sizeof(uint32_t));
                        *marco_ptr = (uint32_t)marco;
                        list_add(tabla_paginas, marco_ptr);
                        log_info(memoria_logger, "Marco %d asignado a la pagina %d", marco, i);
                    } else {
                        log_error(memoria_logger, "No hay marcos libres disponibles");
                        break;
                    }
                }
            } else if (paginas_necesarias < paginas_actuales) {
                // Disminuir tamaño --> liberar marcos
                log_info(memoria_logger, "Reduccion del Proceso");
                log_info(memoria_logger, "PID: %d - Tamaño Actual: %d - Tamaño a Reducir: %d", pid_resize, paginas_actuales * TAM_PAGINA, tamanio);
                for (int i = paginas_actuales - 1; i >= paginas_necesarias; i--) {
                    uint32_t* marco_ptr = list_remove(tabla_paginas, i);
                    int marco = *marco_ptr;
                    liberar_marco(marcos_ocupados, marco);
                    log_info(memoria_logger, "Marco %d liberado de la pagina %d", marco, i);
                    free(marco_ptr);
                }
            }
            // Si no hay Out of memory, CPU recibe un -1 en vez del PID del proceso
            send_out_of_memory(fd_cpu, -1);
            break;
        case ESCRIBIR_MEMORIA:
            uint8_t pid_a_escribir;
            uint32_t direccion_fisica_escribir, tamanio_a_escribir;
            void* datos_escribir;
            bool error_escritura = false;
            if (!recv_escribir_memoria(fd_cpu, &pid_a_escribir, &direccion_fisica_escribir, &datos_escribir, &tamanio_a_escribir)) {
                log_error(memoria_logger, "Hubo un error al recibir la instruccion de escribir memoria.");
                break;
            }

            void* datos_original = datos_escribir; // Guardar el puntero original para liberarlo despues

            usleep(RETARDO_RESPUESTA * 1000);
            log_info(memoria_logger, "Acceso a espacio de usuario:");
            log_info(memoria_logger, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño: %d", pid_a_escribir, direccion_fisica_escribir, tamanio_a_escribir);

            // Buscar marco
            t_list* tabla_paginas_actual_escribir = list_get(tabla_paginas_por_proceso, pid_a_escribir);
            int marco_asignado_escribir = floor(direccion_fisica_escribir / TAM_PAGINA);
            int pagina_actual_escribir = obtener_pagina_asignada(pid_a_escribir, marco_asignado_escribir, tabla_paginas_actual_escribir);
            if (pagina_actual_escribir == -1) {
                log_error(memoria_logger, "No se encontro un marco asignado para la pagina %d del PID: %d", pagina_actual_escribir, pid_a_escribir);
                send_escritura_ok(fd_cpu, -1);
                error_escritura = true;
                free(datos_original);
                break;
            }

            // Para hacer un seguimiento del desplazamiento dentro de la pagina actual
            uint32_t desplazamiento_actual_escribir = direccion_fisica_escribir % TAM_PAGINA;
            
            log_info(memoria_logger, "Pagina inicial: %d - Desplazamiento inicial: %d", pagina_actual_escribir, desplazamiento_actual_escribir);
            
            while (tamanio_a_escribir > 0) {
                // Calcular el tamaño de datos a escribir en este marco
                uint32_t bytes_disponibles_pagina = TAM_PAGINA - desplazamiento_actual_escribir;
                uint32_t tamanio_a_escribir_actual = min(tamanio_a_escribir, bytes_disponibles_pagina);

                // Escribir los datos en el marco actual
                if (!escribir_memoria(espacio_usuario, marco_asignado_escribir, desplazamiento_actual_escribir, tamanio_a_escribir_actual, datos_escribir)) {
                    log_error(memoria_logger, "Error al escribir en la memoria.");
                    // Error en escritura --> envia -1
                    send_escritura_ok(fd_cpu, -1);
                    error_escritura = true;
                    free(datos_original);
                    break;
                }

                log_info(memoria_logger, "Escritura en memoria exitosa - Marco: %d - Desplazamiento: %d - Tamaño: %d", marco_asignado_escribir, desplazamiento_actual_escribir, tamanio_a_escribir_actual);

                // Actualizar el tamanio restante
                tamanio_a_escribir -= tamanio_a_escribir_actual;
                desplazamiento_actual_escribir += tamanio_a_escribir_actual;

                // Si hay mas datos por escribir, pasar a la siguiente pagina
                if (tamanio_a_escribir > 0) {
                    if (desplazamiento_actual_escribir >= TAM_PAGINA) {
                        pagina_actual_escribir++;
                        marco_asignado_escribir = obtener_marco_asignado(pid_a_escribir, pagina_actual_escribir, tabla_paginas_actual_escribir);
                        if (marco_asignado_escribir == -1) {
                            log_error(memoria_logger, "No se encontro un marco asignado para la pagina %d del PID: %d", pagina_actual_escribir, pid_a_escribir);
                            send_escritura_ok(fd_cpu, -1);
                            error_escritura = true;
                            free(datos_original);
                            break;
                        }
                        desplazamiento_actual_escribir = 0; // Reiniciar el desplazamiento para la nueva pagina
                        datos_escribir += tamanio_a_escribir_actual; // Datos restantes a escribir
                        log_info(memoria_logger, "Pasando a la siguiente pagina: %d", pagina_actual_escribir);
                    }
                }
            }

            // Escritura OK --> envia 1
            if (!error_escritura) {
                log_info(memoria_logger, "Escritura en memoria completada correctamente para el PID: %d", pid_a_escribir);
                send_escritura_ok(fd_cpu, 1);
                free(datos_original);
            }
            break;
        case LEER_MEMORIA:
            uint8_t pid_a_leer;
            uint32_t direccion_fisica_leer, tamanio_a_leer;
            if (!recv_leer_memoria(fd_cpu, &pid_a_leer, &direccion_fisica_leer, &tamanio_a_leer)) {
                log_error(memoria_logger, "Hubo un error al recibir la instruccion de leer memoria.");
                break;
            }

            usleep(RETARDO_RESPUESTA * 1000);
            log_info(memoria_logger, "Acceso a espacio de usuario:");
            log_info(memoria_logger, "PID: %d - Accion: LEER - Direccion fisica: %u - Tamaño: %u", pid_a_leer, direccion_fisica_leer, tamanio_a_leer);

            // Buscar marco
            t_list* tabla_paginas_actual_leer = list_get(tabla_paginas_por_proceso, pid_a_leer);
            int marco_asignado_leer = floor(direccion_fisica_leer / TAM_PAGINA);
            int pagina_actual_leer = obtener_pagina_asignada(pid_a_leer, marco_asignado_leer, tabla_paginas_actual_leer);
            if (pagina_actual_leer == -1) {
                log_error(memoria_logger, "No se encontro un marco asignado para la pagina: %d del proceso PID: %d", pagina_actual_leer, pid_a_leer);
                break;
            }
            uint32_t desplazamiento_actual_leer = direccion_fisica_leer % TAM_PAGINA;

            void* datos_leer = malloc(tamanio_a_leer);

            // Leer los datos
            uint32_t bytes_leidos = 0; // Para llevar la cuenta de los bytes leidos
            while (bytes_leidos < tamanio_a_leer) {
                // Calcular el tamanio de datos a leer en este marco
                uint32_t bytes_disponibles_pagina = TAM_PAGINA - desplazamiento_actual_leer;
                uint32_t tamanio_a_leer_actual = min(tamanio_a_leer - bytes_leidos, bytes_disponibles_pagina);

                // Leer los datos en el marco actual
                if (!leer_memoria(espacio_usuario, marco_asignado_leer, desplazamiento_actual_leer, tamanio_a_leer_actual, datos_leer + bytes_leidos)) {
                    log_error(memoria_logger, "Error al leer la memoria.");
                    free(datos_leer);
                    // send_valor_memoria(fd_cpu, NULL, 0); // Envia datos nulos en caso de error --> Revisar
                    break;
                }

                // Actualizar la cantidad de bytes leidos y el desplazamiento
                bytes_leidos += tamanio_a_leer_actual;
                desplazamiento_actual_leer = 0; // Desplazamiento se reinicia para la proxima pagina

                // Si hay mas datos por leer, pasar a la siguiente pagina
                if (bytes_leidos < tamanio_a_leer) {
                    pagina_actual_leer++;
                    marco_asignado_leer = obtener_marco_asignado(pid_a_leer, pagina_actual_leer, tabla_paginas_actual_leer);
                    if (marco_asignado_leer == -1) {
                        log_error(memoria_logger, "No se encontro un marco asignado para la pagina %d del proceso %d.", pagina_actual_leer, pid_a_leer);
                        free(datos_leer);
                        // send_valor_memoria(fd_cpu, NULL, 0); // Envia NULL en caso de error --> Revisar
                        break;
                    }
                }
            }

            // Si la lectura se realizo correctamente, enviar los datos al CPU
            if (bytes_leidos == tamanio_a_leer) {
                send_valor_memoria(fd_cpu, direccion_fisica_leer, datos_leer, tamanio_a_leer);
                // Liberar la memoria reservada para los datos leidos
                free(datos_leer);
            }
            break;
        case NUMERO_PAGINA:
            uint8_t pid_pagina;
			uint32_t numero_pagina, desplazamiento;
			if (!recv_num_pagina(fd_cpu, &pid_pagina, &numero_pagina, &desplazamiento)) {
				log_error(memoria_logger, "Hubo un error al recibir el numero de pagina");
				break;
			}

            usleep(RETARDO_RESPUESTA * 1000);

			t_list* tabla_paginas_marco = list_get(tabla_paginas_por_proceso, pid_pagina);
            uint32_t* marco_ptr = (uint32_t*)list_get(tabla_paginas_marco, numero_pagina);
            uint32_t num_marco = *marco_ptr;

            send_num_marco(fd_cpu, pid_pagina, numero_pagina, num_marco);
            log_info(memoria_logger, "Acceso a Tabla de Paginas:");
            log_info(memoria_logger, "PID: %d - Pagina: %d - Marco: %d", pid_pagina, numero_pagina, num_marco);

            break;
		case -1:
			log_error(memoria_logger, "El CPU se desconecto. Terminando servidor");
			control = 0;
            break;
		default:
			log_warning(memoria_logger, "Operacion desconocida.");
			break;
		}
	}
}