#include "../include/memoria-interfaces.h"

void conexion_memoria_interfaces(void* arg) {
    int fd_interfaz = *(int*)arg;
    free(arg);
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_interfaz);
		switch (cod_op) {
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case ESCRIBIR_MEMORIA:
            uint8_t pid_a_escribir;
            uint32_t direccion_fisica_escribir, tamanio_a_escribir;
            void* datos_escribir;
            bool error_escritura = false;
            if (!recv_escribir_memoria(fd_interfaz, &pid_a_escribir, &direccion_fisica_escribir, &datos_escribir, &tamanio_a_escribir)) {
                log_error(memoria_logger, "Hubo un error al recibir la instruccion de escribir memoria.");
                break;
            }

            void* datos_original = datos_escribir; // Guardar el puntero original para liberarlo despues

            usleep(RETARDO_RESPUESTA * 1000);
            log_info(memoria_logger, "Acceso a espacio de usuario: PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño: %d", pid_a_escribir, direccion_fisica_escribir, tamanio_a_escribir);

            // Buscar marco
            t_list* tabla_paginas_actual_escribir = list_get(tabla_paginas_por_proceso, pid_a_escribir);
            int marco_asignado_escribir = floor(direccion_fisica_escribir / TAM_PAGINA);
            int pagina_actual_escribir = obtener_pagina_asignada(pid_a_escribir, marco_asignado_escribir, tabla_paginas_actual_escribir);
            if (pagina_actual_escribir == -1) {
                log_error(memoria_logger, "No se encontro un marco asignado para la pagina %d del PID: %d", pagina_actual_escribir, pid_a_escribir);
                send_escritura_ok(fd_interfaz, -1);
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
                log_info(memoria_logger, "Bytes disponibles en la pagina: %d, Tamaño a escribir actual: %d", bytes_disponibles_pagina, tamanio_a_escribir_actual);

                // Escribir los datos en el marco actual
                if (!escribir_memoria(espacio_usuario, marco_asignado_escribir, desplazamiento_actual_escribir, tamanio_a_escribir_actual, datos_escribir)) {
                    log_error(memoria_logger, "Error al escribir en la memoria.");
                    // Error en escritura --> envia -1
                    send_escritura_ok(fd_interfaz, -1);
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
                            send_escritura_ok(fd_interfaz, -1);
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
                send_escritura_ok(fd_interfaz, 1);
                free(datos_original);
            }
            break;
		case LEER_MEMORIA:
            uint8_t pid_a_leer;
            uint32_t direccion_fisica_leer, tamanio_a_leer;
            if (!recv_leer_memoria(fd_interfaz, &pid_a_leer, &direccion_fisica_leer, &tamanio_a_leer)) {
                log_error(memoria_logger, "Hubo un error al recibir la instruccion de leer memoria.");
                break;
            }

            usleep(RETARDO_RESPUESTA * 1000);
            log_info(memoria_logger, "Acceso a espacio de usuario: PID: %d - Accion: LEER - Direccion fisica: %u - Tamaño: %u", pid_a_leer, direccion_fisica_leer, tamanio_a_leer);

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
                    // send_valor_memoria(fd_interfaz, NULL, 0); // Envia datos nulos en caso de error --> Revisar
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
                        // send_valor_memoria(fd_interfaz, NULL, 0); // Envia NULL en caso de error --> Revisar
                        break;
                    }
                }
            }

            // Si la lectura se realizo correctamente, enviar los datos al CPU
            if (bytes_leidos == tamanio_a_leer) {
                send_valor_memoria(fd_interfaz, direccion_fisica_leer, datos_leer, tamanio_a_leer);
                // Liberar la memoria reservada para los datos leidos
                free(datos_leer);
            }
            break;
		case -1:
			log_error(memoria_logger, "El cliente de una interfaz IO no se encuentra activo.");
			control = 0;
            break;
		default:
			log_warning(memoria_logger, "Operacion desconocida.");
			break;
		}
	}
}