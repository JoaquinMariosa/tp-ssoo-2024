#include <../include/consola.h>

int contador_procesos;

t_pcb* buscar_pcb(t_queue* cola, uint8_t pid_a_borrar) {
	// Buscar en cola
	t_pcb* pcb_encontrado = NULL;
    t_queue* cola_temp = queue_create();
    while (!queue_is_empty(cola)) {
        t_pcb* pcb = queue_pop(cola);
        if (pcb->pid == pid_a_borrar) {
            pcb_encontrado = pcb;
        } else {
            queue_push(cola_temp, pcb);
        }
    }
	// Reconstruir queue
    while (!queue_is_empty(cola_temp)) {
        queue_push(cola, queue_pop(cola_temp));
    }

	queue_destroy(cola_temp);
	return pcb_encontrado;
}

void finalizar_proceso(uint8_t pid_a_borrar) {
	// Liberar memoria
	send_fin_proceso(fd_memoria, pid_a_borrar);
	// Liberar recursos
	liberar_recursos(pid_a_borrar);
	contador_procesos--;
	sem_post(&semaforoPlanificacion);

	if ((queue_size(colaExec) == 0 && (queue_size(colaNew) + size_all_queues()) > 0)) {
		sem_post(&semaforoPlanificacion2);
	}

	// Si se borra el unico proceso que quedaba entonces detener planificacion
	if ((queue_size(colaNew) + size_all_queues()) == 0) {
		control_planificacion = 0;
	}
}

void buscar_en_queues_y_finalizar(t_pcb* pcb_borrar, uint8_t pid_a_borrar) {
	// Exec
    pthread_mutex_lock(&colaExecMutex);
	pcb_borrar = buscar_pcb(colaExec, pid_a_borrar);
    pthread_mutex_unlock(&colaExecMutex);
	if(pcb_borrar != NULL) {
		log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid_a_borrar, "Exec", "Exit");
		finalizar_proceso(pid_a_borrar);
		free(pcb_borrar->registros);
		free(pcb_borrar);
		return;
	}
	
	// New
    pthread_mutex_lock(&colaNewMutex);
	pcb_borrar = buscar_pcb(colaNew, pid_a_borrar);
    pthread_mutex_unlock(&colaNewMutex);
	if(pcb_borrar != NULL) {
		log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid_a_borrar, "New", "Exit");
		finalizar_proceso(pid_a_borrar);
		free(pcb_borrar->registros);
		free(pcb_borrar);
		return;
	}

	// Ready
    pthread_mutex_lock(&colaReadyMutex);
	pcb_borrar = buscar_pcb(colaReady, pid_a_borrar);
    pthread_mutex_unlock(&colaReadyMutex);
	if(pcb_borrar != NULL) {
		log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid_a_borrar, "Ready", "Exit");
		finalizar_proceso(pid_a_borrar);
		free(pcb_borrar->registros);
		free(pcb_borrar);
		return;
	}

	// Blocked
    pthread_mutex_lock(&colaBlockedMutex);
	pcb_borrar = buscar_pcb(colaBlocked, pid_a_borrar);
    pthread_mutex_unlock(&colaBlockedMutex);
	if(pcb_borrar != NULL) {
		log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid_a_borrar, "Blocked", "Exit");
		finalizar_proceso(pid_a_borrar);
		free(pcb_borrar->registros);
		free(pcb_borrar);
		return;
	}

	// Aux (Ready prioritaria de VRR)
    pthread_mutex_lock(&colaAuxMutex);
	pcb_borrar = buscar_pcb(colaAux, pid_a_borrar);
    pthread_mutex_unlock(&colaAuxMutex);
	if(pcb_borrar != NULL) {
		log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid_a_borrar, "Ready (Prioritario)", "Exit");
		finalizar_proceso(pid_a_borrar);
		free(pcb_borrar->registros);
		free(pcb_borrar);
		return;
	}

	// Blocked de cada recurso
	for(int i = 0; i < list_size(recursos); i++) {
		t_recurso* r = list_get(recursos, i);
		pcb_borrar = buscar_pcb(r->blocked, pid_a_borrar);
		if(pcb_borrar != NULL) {
			log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid_a_borrar, "Blocked", "Exit");
			finalizar_proceso(pid_a_borrar);
			free(pcb_borrar->registros);
			free(pcb_borrar);
			return;
		}
	}
}

void listar_procesos(t_queue* cola, pthread_mutex_t mutex, char* estado) {
    pthread_mutex_lock(&mutex);
    if(queue_is_empty(cola)) {  
    	log_info(kernel_logger, "Procesos en %s: -", estado);
    } else {
		char lista_pids[1024] = "";
        t_list* elements = cola->elements;
        for(int i = 0; i < list_size(elements); i++) {
            t_pcb* pcb = (t_pcb*) list_get(elements, i);
			char pid_str[12];  // Para convertir pid a string
            snprintf(pid_str, sizeof(pid_str), "%d - ", pcb->pid);
            strcat(lista_pids, pid_str);
        }
		lista_pids[strlen(lista_pids) - 2] = '\0';
    	log_info(kernel_logger, "Procesos en %s: %s", estado, lista_pids);
    }
    pthread_mutex_unlock(&mutex);
}

// Se tiene que buscar los bloqueados por IO y por recursos
void listar_procesos_blocked() {
    // Buffer para construir el mensaje de log
    char lista_pids[1024] = "Procesos en Blocked:";
    char pid_str[128];

    // Loguear procesos bloqueados por IO
    pthread_mutex_lock(&colaBlockedMutex);
    if(!queue_is_empty(colaBlocked)) {
        t_list* elements = colaBlocked->elements;
        for(int i = 0; i < list_size(elements); i++) {
            t_pcb* pcb = (t_pcb*) list_get(elements, i);
            snprintf(pid_str, sizeof(pid_str), " %d -", pcb->pid);
            strcat(lista_pids, pid_str);
        }
    }
    pthread_mutex_unlock(&colaBlockedMutex);

    // Loguear procesos bloqueados por cada recurso
    for(int i = 0; i < list_size(recursos); i++) {
        t_recurso* recurso = list_get(recursos, i);
        if(!queue_is_empty(recurso->blocked)) {
            t_list* elements = recurso->blocked->elements;
            for(int j = 0; j < list_size(elements); j++) {
                t_pcb* pcb = (t_pcb*) list_get(elements, j);
                snprintf(pid_str, sizeof(pid_str), " %d -", pcb->pid);
                strcat(lista_pids, pid_str);
            }
        }
    }

	if (strlen(lista_pids) == 20) { // Procesos en Blocked: --> nada mas
    	log_info(kernel_logger, "Procesos en Blocked: -");
		return;
	}

	lista_pids[strlen(lista_pids) - 1] = '\0';
    log_info(kernel_logger, "%s", lista_pids);
}

void iniciar_planificacion() {
    pthread_t planificacion;
	if(strcmp(ALGORITMO_PLANIFICACION, "FIFO") == 0) {
		pthread_create(&planificacion, NULL, (void *)planificacionFIFO, NULL);
	} else if(strcmp(ALGORITMO_PLANIFICACION, "RR") == 0) {
		pthread_create(&planificacion, NULL, (void *)planificacionRR, NULL);
	} else if(strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
		pthread_create(&planificacion, NULL, (void *)planificacionVRR, NULL);
	}
	pthread_detach(planificacion);
}

void atender_instruccion (char* leido) {

	char** comando_consola = string_split(leido, " ");

	if(strcmp(comando_consola[0], "HELP") == 0) {
		printf( "\nInstrucciones:\n"
				"EJECUTAR_SCRIPT [path]\n"
				"INICIAR_PROCESO [path]\n"
				"FINALIZAR_PROCESO [pid]\n"
				"DETENER_PLANIFICACION\n"
				"INICIAR_PLANIFICACION\n"
				"MULTIPROGRAMACION [valor]\n"
				"PROCESO_ESTADO\n\n");
	} else if(strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0 || strcmp(comando_consola[0], "ES") == 0) {
		log_info(kernel_logger, "Path del script: %s", comando_consola[1]);

		// Concatenar con path de carpeta scripts-pruebas
		uint32_t MAX_LENGTH = 256;
		char* path = malloc(MAX_LENGTH);
		strcpy(path, "/home/utnso/so-deploy/tp-2024-1c-ChacoForSystem");
   	 	strcat(path, comando_consola[1]);

		FILE* archivo;
		archivo = fopen(path, "rt");

		if (archivo == NULL) {
			perror("Error abriendo archivo");
			exit(EXIT_FAILURE);
		}

		char* linea = malloc(MAX_LENGTH);

		log_info(kernel_logger, "Leyendo el archivo");
		while (fgets(linea, MAX_LENGTH, archivo) != NULL) {
			char* comando = malloc(strlen(linea) + 1); // +1 para el terminador nulo
			strcpy(comando, linea);
			string_trim_right(&comando);
			log_info(kernel_logger, "Comando: %s", comando);
			atender_instruccion(comando);
			free(comando);
		}

		fclose(archivo); // Cerrar el archivo
		free(linea); // Libero memoria
    	free(path); // Liberar memoria del path
	} else if(strcmp(comando_consola[0], "INICIAR_PROCESO") == 0 || strcmp(comando_consola[0], "IPRO") == 0) {
		t_pcb* pcb = crear_pcb();
		send_pcb(fd_cpu_dispatch, pcb);
		log_info(kernel_logger, "Se crea el proceso %d en NEW", pcb->pid);

        pthread_mutex_lock(&colaNewMutex);
		queue_push(colaNew, pcb);
        pthread_mutex_unlock(&colaNewMutex);

		t_list* recursos_tomados = list_create();
		list_add(recursos_de_procesos, recursos_tomados);

		send_iniciar_proceso(fd_memoria, pcb->pid, comando_consola[1], strlen(comando_consola[1]) + 1);
		log_info(kernel_logger, "Path enviado: %s", comando_consola[1]);
	} else if(strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0 || strcmp(comando_consola[0], "DP") == 0) {
        log_info(kernel_logger, "Se detiene la planificacion");
        sem_wait(&semaforoPlanificacion2);
    } else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0 || strcmp(comando_consola[0], "IPLAN") == 0) {
		if (control_primera_vez) {
            log_info(kernel_logger, "Comienza la planificacion");
			contador_procesos = 0;
            control_primera_vez = false;
        } else {
            log_info(kernel_logger, "Se retoma la planificacion");
        }
        sem_post(&semaforoPlanificacion);
        sem_post(&semaforoPlanificacion2);
        iniciar_planificacion();
	} else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0 || strcmp(comando_consola[0], "FP") == 0) {
		// Buscar pid en queue --> pid => comando_consola[1]
		uint8_t pid_a_borrar = atoi(comando_consola[1]);
		
		t_pcb* pcb_borrar = malloc(sizeof(t_pcb));
		pcb_borrar->registros = malloc(sizeof(t_registros));

		// Exec --> Enviar señal a CPU si esta ejecutando el proceso a finalizar
		pthread_mutex_lock(&colaExecMutex);
		pcb_borrar = buscar_pcb(colaExec, pid_a_borrar);
		if(pcb_borrar != NULL) {
        	log_info(kernel_logger, "Finaliza el proceso %d - Motivo: %s", pid_a_borrar, "Interrupted by User");
			send_pid_a_borrar(fd_cpu_interrupt, pid_a_borrar);

			// Liberar memoria
			free(pcb_borrar->registros);
			free(pcb_borrar);
			// Liberar cada elemento
			int i = 0;
			while (comando_consola[i] != NULL) {
				free(comando_consola[i]);
				i++;
			}
			// Liberar el array
			free(comando_consola);
			return;
		}
		pthread_mutex_unlock(&colaExecMutex);

		buscar_en_queues_y_finalizar(pcb_borrar, pid_a_borrar);
        log_info(kernel_logger, "Finaliza el proceso %d - Motivo: %s", pid_a_borrar, "Interrupted by User");
	} else if(strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0 || strcmp(comando_consola[0], "MP") == 0){
		int nuevo_grado = atoi(comando_consola[1]);
		GRADO_MULTIPROGRAMACION = nuevo_grado;

		//Falta agregar -> En caso de que se tengan más procesos ejecutando que lo que permite el grado de multiprogramación, no se tomarán acciones sobre los mismos y se
		//esperará su finalización normal.

		log_info(kernel_logger, "Grado de multiprogramacion modificado a %d", GRADO_MULTIPROGRAMACION);
	} else if(strcmp(comando_consola[0], "PROCESO_ESTADO") == 0 || strcmp(comando_consola[0], "PE") == 0){
		log_info(kernel_logger, "Listando procesos por estado:");
		
		listar_procesos(colaNew, colaNewMutex, "New");
		listar_procesos(colaReady, colaReadyMutex, "Ready");
		if(strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
			listar_procesos(colaAux, colaAuxMutex, "Ready (prioritario)"); // Solo en VRR
		}
		listar_procesos(colaExec, colaExecMutex, "Exec");
		listar_procesos_blocked();
    	log_info(kernel_logger, "Procesos en Exit: -");
	} else {
		log_warning(kernel_logger, "ERROR. No se encontro el comando. Escribi HELP si necesitas ayuda con los comandos y sus parametros");
	}

	// Liberar cada elemento
	int i = 0;
    while (comando_consola[i] != NULL) {
        free(comando_consola[i]);
        i++;
    }

    // Liberar el array
    free(comando_consola);
}

void iniciar_consola(t_log* logger) {
    char* leido;
	leido = readline("> ");

	while (strcmp(leido,"") != 0) {
		atender_instruccion(leido);
        free(leido);
		leido = readline("> ");
	}
	
	free(leido);
}