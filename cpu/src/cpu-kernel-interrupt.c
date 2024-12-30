#include "../include/cpu-kernel-interrupt.h"

void conexion_cpu_kernel_interrupt() {
    bool control = 1;
	while (control) {
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op) {
		case MENSAJE:
			log_info(cpu_logger, "Mensaje recibido con el numero");
			break;
		case PAQUETE:
			break;
		case RECIBIR_PID_A_INTERRUMPIR:
			uint8_t pid_a_interrumpir;
			if (!recv_interrupcion(fd_kernel_interrupt, &pid_a_interrumpir)) {
				log_error(cpu_logger, "Hubo un error al recibir el PID a interrumpir");
			}

			pthread_mutex_lock(&pcbEjecutarMutex);
			if (pcb_a_ejecutar != NULL) {
				if(pid_a_interrumpir == pcb_a_ejecutar->pid) {
					pcb_a_ejecutar->flag_int = 2; // 2 --> interrupcion por quantum
					pthread_mutex_unlock(&pcbEjecutarMutex);
					break;
				}
			}
			pthread_mutex_unlock(&pcbEjecutarMutex);
			break;
		case RECIBIR_PID_A_BORRAR:
			uint8_t pid_a_borrar;
			if (!recv_pid_a_borrar(fd_kernel_interrupt, &pid_a_borrar)) {
				log_error(cpu_logger, "Hubo un error al recibir el PID a borrar");
			}

			pthread_mutex_lock(&pcbEjecutarMutex);
			if (pcb_a_ejecutar != NULL) {
				if(pid_a_borrar == pcb_a_ejecutar->pid) {
					pcb_a_ejecutar->flag_int = 3; // 3 --> interrupcion por FINALIZAR_PROCESO
					pthread_mutex_unlock(&pcbEjecutarMutex);
					break;
				}
			}
			pthread_mutex_unlock(&pcbEjecutarMutex);
			break;
		case -1:
			log_error(cpu_logger, "El Kernel (Interrupt) se desconecto. Terminando servidor");
			control = 0;
            break;
		default:
			log_warning(cpu_logger, "Operacion desconocida.");
			break;
		}
	}
}
