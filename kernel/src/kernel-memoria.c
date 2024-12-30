#include "../include/kernel-memoria.h"

void conexion_kernel_memoria() {
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op) {
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case OUT_OF_MEMORY:
			uint8_t pid_oom;
			if(!recv_out_of_memory(fd_memoria, &pid_oom)) {
				log_error(kernel_logger, "Hubo un error al recibir el Out of memory");
			}

			log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid_oom, "Exec", "Exit");
            log_info(kernel_logger, "Finaliza el proceso %d - Motivo: %s", pid_oom, "Out of Memory");

			pthread_mutex_lock(&colaExecMutex);
			if(!queue_is_empty(colaExec)) {
				t_pcb* pcb_a_borrar = queue_pop(colaExec);
				sem_post(&semaforoPlanificacion);
				// Liberar marcos
				send_fin_proceso(fd_memoria, pid_oom);
				// Revisar si otro proceso se puede desbloquear
				liberar_recursos(pid_oom);
				contador_procesos--;

				if ((queue_size(colaExec) == 0 && (queue_size(colaNew) + size_all_queues()) > 0)) {
					sem_post(&semaforoPlanificacion2);
				}

				if ((queue_size(colaNew) + size_all_queues()) == 0) {
					control_planificacion = 0;
				}
				free(pcb_a_borrar->registros);
				free(pcb_a_borrar);
			}
			pthread_mutex_unlock(&colaExecMutex);
			break;
		case -1:
			log_error(kernel_logger, "El servidor de Memoria no se encuentra activo.");
			control = 0;
            break;
		default:
			log_warning(kernel_logger, "Operacion desconocida.");
			break;
		}
	}
}