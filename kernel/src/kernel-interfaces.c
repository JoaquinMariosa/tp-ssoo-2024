#include "../include/kernel-interfaces.h"

pthread_mutex_t reciboFinDeIO = PTHREAD_MUTEX_INITIALIZER;

void conexion_kernel_interfaces(void* arg) {
    uint8_t MAX_LENGTH = 128;
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
        case FIN_IO:
			pthread_mutex_lock(&reciboFinDeIO);
			uint8_t pid_fin_io;
			char* nombre_fin_io = malloc(MAX_LENGTH);
			char* nombre_fin_io_recibido = malloc(MAX_LENGTH);

			// Busca el socket de la interfaz
			// fd_interfaz = buscar_socket_interfaz(listaInterfaces, nombre_interfaz);

			if(!recv_fin_io(fd_interfaz, &pid_fin_io, nombre_fin_io_recibido)) { // falta recibir el pid
				log_error(kernel_logger, "Hubo un error al recibir el fin de una IO");
			}
			strcpy(nombre_fin_io, nombre_fin_io_recibido);
			
			pthread_mutex_lock(&colaBlockedMutex);
			if(!queue_is_empty(colaBlocked)) {
				t_pcb* pcb_recibido = buscar_pcb(colaBlocked, pid_fin_io);
				if(pcb_recibido == NULL) {
					pthread_mutex_unlock(&colaBlockedMutex);
					pthread_mutex_unlock(&reciboFinDeIO);
					free(nombre_fin_io);
					free(nombre_fin_io_recibido);
					break;
				}
				if(strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0) {
					if(pcb_recibido->quantum > 0 && pcb_recibido->quantum < QUANTUM) {
						if (pcb_recibido != NULL) {
            				log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "Blocked", "Ready (Prioridad)");
							pcb_recibido->estado = 'A';
							pcb_recibido->flag_int = 0;
							pthread_mutex_lock(&colaAuxMutex);
							queue_push(colaAux, pcb_recibido);
							pthread_mutex_unlock(&colaAuxMutex);
                        	ingreso_ready_aux(colaAux, colaAuxMutex, "Ready Prioritario");
						}
					} else {
						if (pcb_recibido != NULL) {
            				log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "Blocked", "Ready");
							pcb_recibido->estado = 'R';
							pcb_recibido->flag_int = 0;
							pthread_mutex_lock(&colaReadyMutex);
							queue_push(colaReady, pcb_recibido);
							pthread_mutex_unlock(&colaReadyMutex);
                        	ingreso_ready_aux(colaReady, colaReadyMutex, "Ready");
						}
					}
				} else {
					if (pcb_recibido != NULL) {
            			log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "Blocked", "Ready");
						pcb_recibido->estado = 'R';
						pcb_recibido->flag_int = 0;
						pthread_mutex_lock(&colaReadyMutex);
						queue_push(colaReady, pcb_recibido);
						pthread_mutex_unlock(&colaReadyMutex);
                        ingreso_ready_aux(colaReady, colaReadyMutex, "Ready");
					}
				}

				if (queue_size(colaExec) == 0) {
					sem_post(&semaforoPlanificacion2);
				}
			}
			pthread_mutex_unlock(&colaBlockedMutex);
			pthread_mutex_unlock(&reciboFinDeIO);

			free(nombre_fin_io);
			free(nombre_fin_io_recibido);
			break;
		case -1:
			log_error(kernel_logger, "El cliente de una interfaz IO no se encuentra activo.");
			control = 0;
            break;
		default:
			log_warning(kernel_logger, "Operacion desconocida.");
			break;
		}
	}
}