#include "../include/cpu-kernel-dispatch.h"

t_pcb* pcb_a_ejecutar;
t_list* procesos;

void destruir_pcb(void* ptr_pcb) {
    t_pcb* pcb = (t_pcb*)ptr_pcb;
    // Liberar recursos del PCB
    if (pcb && pcb->registros) {
        free(pcb->registros);
    }
    if (pcb) {
        free(pcb);
    }
}

void conexion_cpu_kernel_dispatch() {
    bool control = 1;
	procesos = list_create();
	while (control) {
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op) {
		case RECIBIR_PCB:
			// Recibo PCB
			t_pcb* pcb = malloc(sizeof(t_pcb));
			pcb->registros = malloc(sizeof(t_registros));
			
			if(!recv_pcb(fd_kernel_dispatch, pcb)) {
				log_error(cpu_logger, "Hubo un error al recibir el PCB del modulo Kernel (Dispatch)");
				free(pcb->registros);
				free(pcb);
				exit(EXIT_FAILURE);
			}

			// Controlo si es nulo
			if (pcb == NULL) {
				log_error(cpu_logger, "El PCB recibido es NULL");
				exit(EXIT_FAILURE);
			}

			list_add_in_index(procesos, pcb->pid, pcb);

			// Printea el PCB
			// log_info(cpu_logger, "PID: %d\nProgram Counter: %d\nEstado: %c\nQuantum: %d\nFlag IO: %d\nRegistros:\nAX: %d, BX: %d, CX: %d, DX: %d\nEAX: %d, EBX: %d, ECX: %d, EDX: %d\nSI: %d, DI: %d",
			// pcb->pid, pcb->pc, pcb->estado, pcb->quantum, pcb->flag_int,
			// pcb->registros->AX, pcb->registros->BX, pcb->registros->CX, pcb->registros->DX,
			// pcb->registros->EAX, pcb->registros->EBX, pcb->registros->ECX, pcb->registros->EDX,
			// pcb->registros->SI, pcb->registros->DI);

            break;
		case RECIBIR_PID:
			// Recibo proceso a ejecutar
			uint8_t pid_a_ejecutar;
			if(!recv_pid(fd_kernel_dispatch, &pid_a_ejecutar)) {
				log_error(cpu_logger, "Hubo un error al recibir el PID.");
			} else {
				log_info(cpu_logger, "PID a ejecutar: %d", pid_a_ejecutar);
			}

			pcb_a_ejecutar = list_get(procesos, pid_a_ejecutar);
			if (pcb_a_ejecutar == NULL) {
				break;
			}
			pcb_a_ejecutar->flag_int = 0; // Por si vuelve de interrupcion

			// Fetch --> pedir la primera instruccion y despues pasa al while en cpu-memoria.c
			send_pc_pid(fd_memoria, pcb_a_ejecutar->pc, pcb_a_ejecutar->pid);
			log_info(cpu_logger, "PID: %d - FETCH - Program Counter: %d", pcb_a_ejecutar->pid, pcb_a_ejecutar->pc);
			break;
		case RECURSOS_OK:
			// Para ver si sigue ejecutando el proceso o si esta bloqueado por falta de recursos
			uint8_t recursos_ok;
			if(!recv_recursos_ok(fd_kernel_dispatch, &recursos_ok)) {
				log_error(cpu_logger, "Hubo un error al recibir el OK de la escritura");
			}

			if(recursos_ok == 1) {
				log_info(cpu_logger, "Recurso OK segui con el proceso");
			} else {
				log_info(cpu_logger, "No hay mas recursos disponibles o el recurso no existe");
				esperando_datos = false;
				free(instruccion);
				free(instruccion_recibida);
				// Liberar el arreglo de la instrucción separada
				for (int i = 0; instruccion_separada[i] != NULL; i++) {
					free(instruccion_separada[i]);
				}
				free(instruccion_separada);
				dictionary_destroy(dictionary_registros);
				break;
			}

			// Ya no se esperan datos
			esperando_datos = false; 
			log_info(cpu_logger, "Datos recibidos");

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
			dictionary_destroy(dictionary_registros);

			// Fetch --> seguir pidiendo instrucciones
			send_pc_pid(fd_memoria, pcb_a_ejecutar->pc, pcb_a_ejecutar->pid);
			log_info(cpu_logger, "PID: %d - FETCH - Program Counter: %d", pcb_a_ejecutar->pid, pcb_a_ejecutar->pc);
			break;
		case -1:
			log_error(cpu_logger, "El Kernel (Dispatch) se desconecto. Terminando servidor");
			control = 0;
            break;
		default:
			log_warning(cpu_logger, "Operacion desconocida.");
			break;
		}
	}
    list_destroy_and_destroy_elements(procesos, destruir_pcb);
}