#include "../include/instrucciones.h"

t_instruccion_pendiente* instruccion_pendiente;
pthread_mutex_t instruccion_pendiente_mutex = PTHREAD_MUTEX_INITIALIZER;

uint32_t mmu(uint32_t dir_logica) {
    // Realizo calculos
    uint32_t numero_pagina = dir_logica / tam_pagina;
    uint32_t desplazamiento = dir_logica - numero_pagina * tam_pagina;

    t_tlb* entrada_tlb = buscar_en_tlb(pcb_a_ejecutar->pid, numero_pagina);
    if (entrada_tlb != NULL) {
        // TLB Hit
        log_info(cpu_logger, "PID: %d - TLB HIT - Pagina: %d", pcb_a_ejecutar->pid, numero_pagina);
        // Mover marco para LRU
        if (strcmp(ALGORITMO_TLB, "LRU") == 0) {
            // El marco mas recientemente usado va a ir al final de la lista (index: list_size(lista_tlb) - 1)
            // mientras que el menos recientemente usado va al principio (index: 0)
            int index = buscar_indice_en_lista(lista_tlb, entrada_tlb);
            if (index != -1) {
                mover_elemento_al_principio(lista_tlb, index);
            }
        }
        return (entrada_tlb->marco * tam_pagina) + desplazamiento;
    } else {
        // TLB Miss
        log_info(cpu_logger, "PID: %d - TLB MISS - Pagina: %d", pcb_a_ejecutar->pid, numero_pagina);
        // send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
        return -1; // Significa que es un TLB Miss y esta esperando recibir el marco
    }
}

void funcion_set(t_dictionary* dictionary_registros, char* registro, int valor) {
    if (strlen(registro) == 3 || !strcmp(registro, "SI") || !strcmp(registro, "DI") || !strcmp(registro, "PC")) {
        uint32_t *r_destino = dictionary_get(dictionary_registros, registro);
        *r_destino = valor;
    } else if (strlen(registro) == 2) {
        uint8_t *r_destino = dictionary_get(dictionary_registros, registro);
        *r_destino = valor;
    }

    pcb_a_ejecutar->pc++;
}

void funcion_sum(t_dictionary* dictionary_registros, char* registro_destino, char* registro_origen) {
    if (strlen(registro_destino) == 3 || !strcmp(registro_destino, "SI") || !strcmp(registro_destino, "DI") || !strcmp(registro_destino, "PC")) {
        uint32_t *r_destino = dictionary_get(dictionary_registros, registro_destino);
        if (strlen(registro_origen) == 3 || !strcmp(registro_origen, "SI") || !strcmp(registro_origen, "DI") || !strcmp(registro_origen, "PC")) {
            uint32_t *r_origen = dictionary_get(dictionary_registros, registro_origen);
            *r_destino += *r_origen;
        } else if (strlen(registro_origen) == 2) {
            uint8_t *r_origen = dictionary_get(dictionary_registros, registro_origen);
            *r_destino += *r_origen;
        }
    } else if (strlen(registro_destino) == 2) {
        uint8_t *r_destino = dictionary_get(dictionary_registros, registro_destino);
        if (strlen(registro_origen) == 3 || !strcmp(registro_origen, "SI") || !strcmp(registro_origen, "DI") || !strcmp(registro_origen, "PC")) {
            uint32_t *r_origen = dictionary_get(dictionary_registros, registro_origen);
            *r_destino += *r_origen;
        } else if (strlen(registro_origen) == 2) {
            uint8_t *r_origen = dictionary_get(dictionary_registros, registro_origen);
            *r_destino += *r_origen;
        }
    }

    pcb_a_ejecutar->pc++;
}

void funcion_sub(t_dictionary* dictionary_registros, char* registro_destino, char* registro_origen) {
    if (strlen(registro_destino) == 3 || !strcmp(registro_destino, "SI") || !strcmp(registro_destino, "DI") || !strcmp(registro_destino, "PC")) {
        uint32_t *r_destino = dictionary_get(dictionary_registros, registro_destino);
        if (strlen(registro_origen) == 3 || !strcmp(registro_origen, "SI") || !strcmp(registro_origen, "DI") || !strcmp(registro_origen, "PC")) {
            uint32_t *r_origen = dictionary_get(dictionary_registros, registro_origen);
            *r_destino -= *r_origen;
        } else if (strlen(registro_origen) == 2) {
            uint8_t *r_origen = dictionary_get(dictionary_registros, registro_origen);
            *r_destino -= *r_origen;
        }
    } else if (strlen(registro_destino) == 2) {
        uint8_t *r_destino = dictionary_get(dictionary_registros, registro_destino);
        if (strlen(registro_origen) == 3 || !strcmp(registro_origen, "SI") || !strcmp(registro_origen, "DI") || !strcmp(registro_origen, "PC")) {
            uint32_t *r_origen = dictionary_get(dictionary_registros, registro_origen);
            *r_destino -= *r_origen;
        } else if (strlen(registro_origen) == 2) {
            uint8_t *r_origen = dictionary_get(dictionary_registros, registro_origen);
            *r_destino -= *r_origen;
        }
    }

    pcb_a_ejecutar->pc++;
}

void funcion_mov_in(t_dictionary* dictionary_registros, char* registro_datos, char* registro_direccion) {
    // Obtener la direccion logica
    uint32_t direccion_fisica;
    if (strlen(registro_direccion) == 3 || !strcmp(registro_direccion, "SI") || !strcmp(registro_direccion, "DI") || !strcmp(registro_direccion, "PC")) {
        uint32_t *dir_logica = dictionary_get(dictionary_registros, registro_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("MOV_IN");
            instruccion_pendiente->registro_datos = strdup(registro_datos);
            instruccion_pendiente->registro_direccion = strdup(registro_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = 0; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            instruccion_pendiente->nombre_interfaz = NULL; // No tiene
            instruccion_pendiente->puntero_archivo = 0; // No tiene
            instruccion_pendiente->nombre_archivo = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            return;
        }
	} else if (strlen(registro_direccion) == 2) {
        uint8_t *dir_logica = dictionary_get(dictionary_registros, registro_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("MOV_IN");
            instruccion_pendiente->registro_datos = strdup(registro_datos);
            instruccion_pendiente->registro_direccion = strdup(registro_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = 0; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            instruccion_pendiente->nombre_interfaz = NULL; // No tiene
            instruccion_pendiente->puntero_archivo = 0; // No tiene
            instruccion_pendiente->nombre_archivo = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            return;
        }
    }

    // TLB hit, continuar con la ejecucion normal de mov_in
    if (strlen(registro_datos) == 3 || !strcmp(registro_datos, "SI") || !strcmp(registro_datos, "DI") || !strcmp(registro_datos, "PC")) {
        uint32_t tamanio_a_leer = sizeof(uint32_t);
		send_leer_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, tamanio_a_leer);
        pthread_mutex_lock(&instruccion_pendiente_mutex);
        instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
        instruccion_pendiente->instruccion = strdup("MOV_IN");
        instruccion_pendiente->registro_datos = strdup(registro_datos);
        instruccion_pendiente->registro_direccion = strdup(registro_direccion);
        instruccion_pendiente->direccion_logica = 0; // Ya se tradujo
        instruccion_pendiente->tamanio = 0; // No tiene
        instruccion_pendiente->datos = NULL; // No tiene
        instruccion_pendiente->nombre_interfaz = NULL; // No tiene
        instruccion_pendiente->puntero_archivo = 0; // No tiene
        instruccion_pendiente->nombre_archivo = NULL; // No tiene
        pthread_mutex_unlock(&instruccion_pendiente_mutex);
	} else if (strlen(registro_datos) == 2) {
        uint32_t tamanio_a_leer = sizeof(uint8_t);
		send_leer_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, tamanio_a_leer);
        pthread_mutex_lock(&instruccion_pendiente_mutex);
        instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
        instruccion_pendiente->instruccion = strdup("MOV_IN");
        instruccion_pendiente->registro_datos = strdup(registro_datos);
        instruccion_pendiente->registro_direccion = strdup(registro_direccion);
        instruccion_pendiente->direccion_logica = 0; // Ya se tradujo
        instruccion_pendiente->tamanio = 0; // No tiene
        instruccion_pendiente->datos = NULL; // No tiene
        instruccion_pendiente->nombre_interfaz = NULL; // No tiene
        instruccion_pendiente->puntero_archivo = 0; // No tiene
        instruccion_pendiente->nombre_archivo = NULL; // No tiene
        pthread_mutex_unlock(&instruccion_pendiente_mutex);
	}

    pcb_a_ejecutar->pc++;
}

void funcion_mov_out(t_dictionary* dictionary_registros, char* registro_direccion, char* registro_datos) {
    // Obtener la direccion logica
    uint32_t direccion_fisica;
    if (strlen(registro_direccion) == 3 || !strcmp(registro_direccion, "SI") || !strcmp(registro_direccion, "DI") || !strcmp(registro_direccion, "PC")) {
        uint32_t *dir_logica = dictionary_get(dictionary_registros, registro_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("MOV_OUT");
            instruccion_pendiente->registro_datos = strdup(registro_datos);
            instruccion_pendiente->registro_direccion = strdup(registro_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = 0; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            instruccion_pendiente->nombre_interfaz = NULL; // No tiene
            instruccion_pendiente->puntero_archivo = 0; // No tiene
            instruccion_pendiente->nombre_archivo = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            return;
        }
	} else if (strlen(registro_direccion) == 2) {
        uint8_t *dir_logica = dictionary_get(dictionary_registros, registro_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("MOV_OUT");
            instruccion_pendiente->registro_datos = strdup(registro_datos);
            instruccion_pendiente->registro_direccion = strdup(registro_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = 0; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            instruccion_pendiente->nombre_interfaz = NULL; // No tiene
            instruccion_pendiente->puntero_archivo = 0; // No tiene
            instruccion_pendiente->nombre_archivo = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            return;
        }
    }
    
    // TLB hit, continuar con la ejecucion normal de mov_out
    if (strlen(registro_datos) == 3 || !strcmp(registro_datos, "SI") || !strcmp(registro_datos, "DI") || !strcmp(registro_datos, "PC")) {
		uint32_t *reg_datos = dictionary_get(dictionary_registros, registro_datos);
		uint32_t tamanio_a_escribir = sizeof(uint32_t);
        log_info(cpu_logger, "PID: %d - Accion: ESCRIBIR - Direccion Fisica: %d - Valor: %d", pcb_a_ejecutar->pid, direccion_fisica, *reg_datos);
		send_escribir_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, reg_datos, tamanio_a_escribir);
	} else if (strlen(registro_datos) == 2) {
		uint8_t *reg_datos = dictionary_get(dictionary_registros, registro_datos);
		uint32_t tamanio_a_escribir = sizeof(uint8_t);
        log_info(cpu_logger, "PID: %d - Accion: ESCRIBIR - Direccion Fisica: %d - Valor: %d", pcb_a_ejecutar->pid, direccion_fisica, *reg_datos);
	    send_escribir_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica, reg_datos, tamanio_a_escribir);
	}
}

void funcion_jnz(t_dictionary* dictionary_registros, char* registro, uint32_t valor_pc) {
    if (strlen(registro) == 3 || !strcmp(registro, "SI") || !strcmp(registro, "DI") || !strcmp(registro, "PC")) {
        uint32_t *r_registro = dictionary_get(dictionary_registros, registro);
        if (*r_registro != 0) {
            pcb_a_ejecutar->pc = valor_pc;
        } else {
            pcb_a_ejecutar->pc++;
        }
    } else if (strlen(registro) == 2) {
        uint8_t *r_registro = dictionary_get(dictionary_registros, registro);
        if(*r_registro != 0) { 
            pcb_a_ejecutar->pc = valor_pc;
        } else {
            pcb_a_ejecutar->pc++;
        }
    }
}

void funcion_resize(uint32_t tamanio) {
    if(tamanio == 0) {
        int i = 0;
        while (i < list_size(lista_tlb)) {
            t_tlb* entrada_a_buscar = list_get(lista_tlb, i);
            if (entrada_a_buscar->pid == pcb_a_ejecutar->pid) {
                t_tlb* entrada_a_borrar = list_remove(lista_tlb, i);
                free(entrada_a_borrar);
            } else {
                i++;
            }
        }
    }

    send_tamanio(fd_memoria, tamanio, pcb_a_ejecutar->pid);
    pcb_a_ejecutar->pc++;
}

void funcion_copy_string(t_dictionary* dictionary_registros, uint32_t tamanio) {
    // Contiene la direccion logica de memoria de origen desde donde se va a copiar un string.
    uint32_t *reg_si = dictionary_get(dictionary_registros, "SI");
    uint32_t direccion_fisica_si = mmu(*reg_si);
    if (direccion_fisica_si == -1) {
        // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
        pthread_mutex_lock(&instruccion_pendiente_mutex);
        instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
        instruccion_pendiente->instruccion = strdup("COPY_STRING");
        instruccion_pendiente->direccion_logica = *reg_si;
        instruccion_pendiente->registro_datos = strdup("SI");
        instruccion_pendiente->tamanio = tamanio;
        instruccion_pendiente->registro_direccion = NULL; // No tiene
        instruccion_pendiente->datos = NULL; // No tiene
        instruccion_pendiente->nombre_interfaz = NULL; // No tiene
        instruccion_pendiente->puntero_archivo = 0; // No tiene
        instruccion_pendiente->nombre_archivo = NULL; // No tiene
        uint32_t numero_pagina = *reg_si / tam_pagina;
        uint32_t desplazamiento = *reg_si - numero_pagina * tam_pagina;
        send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
        pthread_mutex_unlock(&instruccion_pendiente_mutex);
        return;
    }

    // TLB hit
    send_leer_memoria(fd_memoria, pcb_a_ejecutar->pid, direccion_fisica_si, tamanio);
    
    pthread_mutex_lock(&instruccion_pendiente_mutex);
    instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
    instruccion_pendiente->instruccion = strdup("COPY_STRING");
    instruccion_pendiente->direccion_logica = *reg_si;
    instruccion_pendiente->registro_datos = strdup("SI");
    instruccion_pendiente->tamanio = tamanio;
    instruccion_pendiente->registro_direccion = NULL; // No tiene
    instruccion_pendiente->datos = NULL; // No tiene
    instruccion_pendiente->nombre_interfaz = NULL; // No tiene
    instruccion_pendiente->puntero_archivo = 0; // No tiene
    instruccion_pendiente->nombre_archivo = NULL; // No tiene
    pthread_mutex_unlock(&instruccion_pendiente_mutex);

    // Sigue cuando recibe el marco (si es necesario) o cuando recibe el valor de memoria (string)
}

void funcion_wait(char* recurso) {
    pcb_a_ejecutar->pc++;
    send_wait(fd_kernel_dispatch, pcb_a_ejecutar, recurso, strlen(recurso) + 1);
}

void funcion_signal(char* recurso) {
    pcb_a_ejecutar->pc++;
    send_signal(fd_kernel_dispatch, pcb_a_ejecutar, recurso, strlen(recurso) + 1);
}

void funcion_io_gen_sleep(char* interfaz, uint32_t unidades_trabajo) {
    pcb_a_ejecutar->flag_int = 1; // Cuando envio el Contexto de ejecucion al Kernel, sabe que el proceso fue interrumpido por una interfaz IO
    pcb_a_ejecutar->pc++;
    // Enviar el pcb como contexto de ejecucion directamente en la funcion de send_io_gen_sleep
    send_io_gen_sleep(fd_kernel_dispatch, pcb_a_ejecutar, unidades_trabajo, interfaz, strlen(interfaz) + 1); // Envia pcb y el nombre de la interfaz
}

void funcion_io_stdin_read(t_dictionary* dictionary_registros, char* interfaz, char* registro_direccion, char* registro_tamanio) {
    // Cuando envio el Contexto de ejecucion al Kernel, sabe que el proceso fue interrumpido por una interfaz IO
    pcb_a_ejecutar->flag_int = 1;
    pcb_a_ejecutar->pc++;

    // Obtener tamanio maximo
    uint32_t tamanio_maximo;
    if (strlen(registro_tamanio) == 3 || !strcmp(registro_tamanio, "SI") || !strcmp(registro_tamanio, "DI") || !strcmp(registro_tamanio, "PC")) {
		uint32_t *reg_tamanio = dictionary_get(dictionary_registros, registro_tamanio);
        tamanio_maximo = *reg_tamanio;
	} else if (strlen(registro_tamanio) == 2) {
		uint8_t *reg_tamanio = dictionary_get(dictionary_registros, registro_tamanio);
        tamanio_maximo = *reg_tamanio;
	}

    // Traducir direccion logica a fisica
    uint32_t direccion_fisica;
    if (strlen(registro_direccion) == 3 || !strcmp(registro_direccion, "SI") || !strcmp(registro_direccion, "DI") || !strcmp(registro_direccion, "PC")) {
        uint32_t *dir_logica = dictionary_get(dictionary_registros, registro_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("IO_STDIN_READ");
            instruccion_pendiente->registro_direccion = strdup(registro_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = tamanio_maximo;
            instruccion_pendiente->nombre_interfaz = strdup(interfaz);
            instruccion_pendiente->registro_datos = NULL; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            instruccion_pendiente->puntero_archivo = 0; // No tiene
            instruccion_pendiente->nombre_archivo = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            // Espera recibir el marco
            esperando_datos = true;
            return;
        }
	} else if (strlen(registro_direccion) == 2) {
        uint8_t *dir_logica = dictionary_get(dictionary_registros, registro_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("IO_STDIN_READ");
            instruccion_pendiente->registro_direccion = strdup(registro_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = tamanio_maximo;
            instruccion_pendiente->nombre_interfaz = strdup(interfaz);
            instruccion_pendiente->registro_datos = NULL; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            instruccion_pendiente->puntero_archivo = 0; // No tiene
            instruccion_pendiente->nombre_archivo = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            // Espera recibir el marco
            esperando_datos = true;
            return;
        }
    }

    // TLB hit
    // Enviar el pcb como contexto de ejecucion directamente en la funcion de send_io_stdin_read
    send_io_stdin_read(fd_kernel_dispatch, pcb_a_ejecutar, direccion_fisica, tamanio_maximo, interfaz, strlen(interfaz) + 1);
}

void funcion_io_stdout_write(t_dictionary* dictionary_registros, char* interfaz, char* registro_direccion, char* registro_tamanio) {
    // Cuando envio el Contexto de ejecucion al Kernel, sabe que el proceso fue interrumpido por una interfaz IO
    pcb_a_ejecutar->flag_int = 1;
    pcb_a_ejecutar->pc++;

    // Obtener tamanio maximo
    uint32_t tamanio_maximo;
    if (strlen(registro_tamanio) == 3 || !strcmp(registro_tamanio, "SI") || !strcmp(registro_tamanio, "DI") || !strcmp(registro_tamanio, "PC")) {
		uint32_t *reg_tamanio = dictionary_get(dictionary_registros, registro_tamanio);
        tamanio_maximo = *reg_tamanio;
	} else if (strlen(registro_tamanio) == 2) {
		uint8_t *reg_tamanio = dictionary_get(dictionary_registros, registro_tamanio);
        tamanio_maximo = *reg_tamanio;
	}

    // Traducir direccion logica a fisica
    uint32_t direccion_fisica;
    if (strlen(registro_direccion) == 3 || !strcmp(registro_direccion, "SI") || !strcmp(registro_direccion, "DI") || !strcmp(registro_direccion, "PC")) {
        uint32_t *dir_logica = dictionary_get(dictionary_registros, registro_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("IO_STDOUT_WRITE");
            instruccion_pendiente->registro_direccion = strdup(registro_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = tamanio_maximo;
            instruccion_pendiente->nombre_interfaz = strdup(interfaz);
            instruccion_pendiente->registro_datos = NULL; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            instruccion_pendiente->puntero_archivo = 0; // No tiene
            instruccion_pendiente->nombre_archivo = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            // Espera recibir el marco
            esperando_datos = true;
            return;
        }
	} else if (strlen(registro_direccion) == 2) {
        uint8_t *dir_logica = dictionary_get(dictionary_registros, registro_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("IO_STDOUT_WRITE");
            instruccion_pendiente->registro_direccion = strdup(registro_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = tamanio_maximo;
            instruccion_pendiente->nombre_interfaz = strdup(interfaz);
            instruccion_pendiente->registro_datos = NULL; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            instruccion_pendiente->puntero_archivo = 0; // No tiene
            instruccion_pendiente->nombre_archivo = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            // Espera recibir el marco
            esperando_datos = true;
            return;
        }
    }

    // TLB hit
    // Enviar el pcb como contexto de ejecucion directamente en la funcion de send_io_stdout_write
    send_io_stdout_write(fd_kernel_dispatch, pcb_a_ejecutar, direccion_fisica, tamanio_maximo, interfaz, strlen(interfaz) + 1);
}

void funcion_io_fs_create(char* interfaz, char* nombre_archivo) {
    // Cuando envio el Contexto de ejecucion al Kernel, sabe que el proceso fue interrumpido por una interfaz IO
    pcb_a_ejecutar->flag_int = 1;
    pcb_a_ejecutar->pc++;

    send_io_fs_create(fd_kernel_dispatch, pcb_a_ejecutar, nombre_archivo, strlen(nombre_archivo) + 1, interfaz, strlen(interfaz) + 1);
}

void funcion_io_fs_delete(char* interfaz, char* nombre_archivo) {
    // Cuando envio el Contexto de ejecucion al Kernel, sabe que el proceso fue interrumpido por una interfaz IO
    pcb_a_ejecutar->flag_int = 1;
    pcb_a_ejecutar->pc++;

    send_io_fs_delete(fd_kernel_dispatch, pcb_a_ejecutar, nombre_archivo, strlen(nombre_archivo) + 1, interfaz, strlen(interfaz) + 1);
}

void funcion_io_fs_truncate(t_dictionary* dictionary_registros, char* interfaz, char* nombre_archivo, char* registro_tamanio) {
    // Cuando envio el Contexto de ejecucion al Kernel, sabe que el proceso fue interrumpido por una interfaz IO
    pcb_a_ejecutar->flag_int = 1;
    pcb_a_ejecutar->pc++;

    uint32_t tamanio;
    if (strlen(registro_tamanio) == 3 || !strcmp(registro_tamanio, "SI") || !strcmp(registro_tamanio, "DI") || !strcmp(registro_tamanio, "PC")) {
        uint32_t *r_tamanio = dictionary_get(dictionary_registros, registro_tamanio);
        tamanio = *r_tamanio;
    } else if (strlen(registro_tamanio) == 2) {
        uint8_t *r_tamanio = dictionary_get(dictionary_registros, registro_tamanio);
        tamanio = *r_tamanio;
    }

    send_io_fs_truncate(fd_kernel_dispatch, pcb_a_ejecutar, tamanio, nombre_archivo, strlen(nombre_archivo) + 1, interfaz, strlen(interfaz) + 1);
}

void funcion_io_fs_write(t_dictionary* dictionary_registros, char* interfaz, char* nombre_archivo, char* reg_direccion, char* reg_tamanio, char* reg_puntero_archivo) {
    // Cuando envio el Contexto de ejecucion al Kernel, sabe que el proceso fue interrumpido por una interfaz IO
    pcb_a_ejecutar->flag_int = 1;
    pcb_a_ejecutar->pc++;

    uint32_t puntero_archivo = 0;
    if (strlen(reg_puntero_archivo) == 3 || !strcmp(reg_puntero_archivo, "SI") || !strcmp(reg_puntero_archivo, "DI") || !strcmp(reg_puntero_archivo, "PC")) {
        uint32_t *r_ptr_arch = dictionary_get(dictionary_registros, reg_puntero_archivo);
        puntero_archivo = *r_ptr_arch;
    } else if (strlen(reg_tamanio) == 2) {
        uint8_t *r_ptr_arch = dictionary_get(dictionary_registros, reg_puntero_archivo);
        puntero_archivo = *r_ptr_arch;
    }

    uint32_t tamanio_write = 0;
    if (strlen(reg_tamanio) == 3 || !strcmp(reg_tamanio, "SI") || !strcmp(reg_tamanio, "DI") || !strcmp(reg_tamanio, "PC")) {
        uint32_t *r_tamanio = dictionary_get(dictionary_registros, reg_tamanio);
        tamanio_write = *r_tamanio;
    } else if (strlen(reg_tamanio) == 2) {
        uint8_t *r_tamanio = dictionary_get(dictionary_registros, reg_tamanio);
        tamanio_write = *r_tamanio;
    }

    // Traducir direccion logica a fisica
    uint32_t direccion_fisica = 0;
    if (strlen(reg_direccion) == 3 || !strcmp(reg_direccion, "SI") || !strcmp(reg_direccion, "DI") || !strcmp(reg_direccion, "PC")) {
        uint32_t *dir_logica = dictionary_get(dictionary_registros, reg_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("IO_FS_WRITE");
            instruccion_pendiente->registro_direccion = strdup(reg_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = tamanio_write;
            instruccion_pendiente->puntero_archivo = puntero_archivo;
            instruccion_pendiente->nombre_interfaz = strdup(interfaz);
            instruccion_pendiente->nombre_archivo = strdup(nombre_archivo);
            instruccion_pendiente->registro_datos = NULL; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            // Espera recibir el marco
            esperando_datos = true;
            return;
        }
	} else if (strlen(reg_direccion) == 2) {
        uint8_t *dir_logica = dictionary_get(dictionary_registros, reg_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("IO_FS_WRITE");
            instruccion_pendiente->registro_direccion = strdup(reg_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = tamanio_write;
            instruccion_pendiente->puntero_archivo = puntero_archivo;
            instruccion_pendiente->nombre_interfaz = strdup(interfaz);
            instruccion_pendiente->nombre_archivo = strdup(nombre_archivo);
            instruccion_pendiente->registro_datos = NULL; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            // Espera recibir el marco
            esperando_datos = true;
            return;
        }
    }

    send_io_fs_write(fd_kernel_dispatch, pcb_a_ejecutar, tamanio_write, direccion_fisica, puntero_archivo, nombre_archivo, strlen(nombre_archivo) + 1, interfaz, strlen(interfaz) + 1);
}

void funcion_io_fs_read(t_dictionary* dictionary_registros, char* interfaz, char* nombre_archivo, char* reg_direccion, char* reg_tamanio, char* reg_puntero_archivo) {
    // Cuando envio el Contexto de ejecucion al Kernel, sabe que el proceso fue interrumpido por una interfaz IO
    pcb_a_ejecutar->flag_int = 1;
    pcb_a_ejecutar->pc++;

    uint32_t puntero_archivo;
    if (strlen(reg_puntero_archivo) == 3 || !strcmp(reg_puntero_archivo, "SI") || !strcmp(reg_puntero_archivo, "DI") || !strcmp(reg_puntero_archivo, "PC")) {
        uint32_t *r_ptr_arch = dictionary_get(dictionary_registros, reg_puntero_archivo);
        puntero_archivo = *r_ptr_arch;
    } else if (strlen(reg_tamanio) == 2) {
        uint8_t *r_ptr_arch = dictionary_get(dictionary_registros, reg_puntero_archivo);
        puntero_archivo = *r_ptr_arch;
    }

    uint32_t tamanio_read;
    if (strlen(reg_tamanio) == 3 || !strcmp(reg_tamanio, "SI") || !strcmp(reg_tamanio, "DI") || !strcmp(reg_tamanio, "PC")) {
        uint32_t *r_tamanio = dictionary_get(dictionary_registros, reg_tamanio);
        tamanio_read = *r_tamanio;
    } else if (strlen(reg_tamanio) == 2) {
        uint8_t *r_tamanio = dictionary_get(dictionary_registros, reg_tamanio);
        tamanio_read = *r_tamanio;
    }

    // Traducir direccion logica a fisica
    uint32_t direccion_fisica;
    if (strlen(reg_direccion) == 3 || !strcmp(reg_direccion, "SI") || !strcmp(reg_direccion, "DI") || !strcmp(reg_direccion, "PC")) {
        uint32_t *dir_logica = dictionary_get(dictionary_registros, reg_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("IO_FS_READ");
            instruccion_pendiente->registro_direccion = strdup(reg_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = tamanio_read;
            instruccion_pendiente->puntero_archivo = puntero_archivo;
            instruccion_pendiente->nombre_interfaz = strdup(interfaz);
            instruccion_pendiente->nombre_archivo = strdup(nombre_archivo);
            instruccion_pendiente->registro_datos = NULL; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            // Espera recibir el marco
            esperando_datos = true;
            return;
        }
	} else if (strlen(reg_direccion) == 2) {
        uint8_t *dir_logica = dictionary_get(dictionary_registros, reg_direccion);
        direccion_fisica = mmu(*dir_logica);
        if (direccion_fisica == -1) {
            // TLB miss, guardar la instruccion pendiente y esperar a recibir el marco
            pthread_mutex_lock(&instruccion_pendiente_mutex);
            instruccion_pendiente = malloc(sizeof(t_instruccion_pendiente));
            instruccion_pendiente->instruccion = strdup("IO_FS_READ");
            instruccion_pendiente->registro_direccion = strdup(reg_direccion);
            instruccion_pendiente->direccion_logica = *dir_logica;
            instruccion_pendiente->tamanio = tamanio_read;
            instruccion_pendiente->puntero_archivo = puntero_archivo;
            instruccion_pendiente->nombre_interfaz = strdup(interfaz);
            instruccion_pendiente->nombre_archivo = strdup(nombre_archivo);
            instruccion_pendiente->registro_datos = NULL; // No tiene
            instruccion_pendiente->datos = NULL; // No tiene
            uint32_t numero_pagina = *dir_logica / tam_pagina;
            uint32_t desplazamiento = *dir_logica - numero_pagina * tam_pagina;
            send_num_pagina(fd_memoria, pcb_a_ejecutar->pid, numero_pagina, desplazamiento);
            pthread_mutex_unlock(&instruccion_pendiente_mutex);
            // Espera recibir el marco
            esperando_datos = true;
            return;
        }
    }

    send_io_fs_read(fd_kernel_dispatch, pcb_a_ejecutar, tamanio_read, direccion_fisica, puntero_archivo, nombre_archivo, strlen(nombre_archivo) + 1, interfaz, strlen(interfaz) + 1);
}

void funcion_exit() {
    send_pid_a_borrar(fd_kernel_dispatch, pcb_a_ejecutar->pid);
    free(pcb_a_ejecutar->registros);
    free(pcb_a_ejecutar);
    pcb_a_ejecutar = NULL;
}