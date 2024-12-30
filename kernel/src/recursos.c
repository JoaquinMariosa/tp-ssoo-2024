#include "../include/recursos.h"

t_list* recursos_de_procesos;
t_list* recursos;

void iniciar_recursos() {
    recursos = list_create();
    recursos_de_procesos = list_create();

    for(int i = 0; RECURSOS[i] != NULL; i++) {
        t_recurso* recurso = malloc(sizeof(t_recurso));
        recurso->nombre = strdup(RECURSOS[i]);
        recurso->instancias = atoi(INSTANCIAS_RECURSOS[i]);
        recurso->blocked = queue_create();
        list_add(recursos, recurso);
    }
}

void liberar_recursos(uint8_t pid) {
    t_list* rec_tom = list_get(recursos_de_procesos, pid);
	for(int i = 0; i < list_size(rec_tom); i++) {
		t_recurso* r = list_get(rec_tom, i);
        r->instancias++;
		if(!queue_is_empty(r->blocked)) {
            t_pcb* pcb_desbloqueado = queue_pop(r->blocked);
            if(strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
				if(pcb_desbloqueado->quantum > 0 && pcb_desbloqueado->quantum < QUANTUM) {
					if (pcb_desbloqueado != NULL) {
            			log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_desbloqueado->pid, "Blocked", "Ready (Prioridad)");
						pcb_desbloqueado->estado = 'A';
						pcb_desbloqueado->flag_int = 0;
						pthread_mutex_lock(&colaAuxMutex);
						queue_push(colaAux, pcb_desbloqueado);
						pthread_mutex_unlock(&colaAuxMutex);
                        ingreso_ready_aux(colaAux, colaAuxMutex, "Ready Prioritario");
					}
				} else {
					if (pcb_desbloqueado != NULL) {
            			log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_desbloqueado->pid, "Blocked", "Ready");
						pcb_desbloqueado->estado = 'R';
						pcb_desbloqueado->flag_int = 0;
						pthread_mutex_lock(&colaReadyMutex);
						queue_push(colaReady, pcb_desbloqueado);
						pthread_mutex_unlock(&colaReadyMutex);
                        ingreso_ready_aux(colaReady, colaReadyMutex, "Ready");
					}
				}
			} else {
				if (pcb_desbloqueado != NULL) {
            		log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_desbloqueado->pid, "Blocked", "Ready");
					pcb_desbloqueado->estado = 'R';
					pcb_desbloqueado->flag_int = 0;
					pthread_mutex_lock(&colaReadyMutex);
					queue_push(colaReady, pcb_desbloqueado);
					pthread_mutex_unlock(&colaReadyMutex);
                    ingreso_ready_aux(colaReady, colaReadyMutex, "Ready");
				}
			}
	    }
    }
}