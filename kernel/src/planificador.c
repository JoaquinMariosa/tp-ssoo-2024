#include "../include/planificador.h"

pthread_mutex_t colaNewMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t colaReadyMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t colaBlockedMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t colaExecMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t colaAuxMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t quantum_thread;

t_temporal* tiempo_vrr;

int control_planificacion;

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

void* quantum(void* arg) {
    t_pcb* pcb = (t_pcb*)arg;
    usleep(pcb->quantum * 1000);
    if(pcb == NULL) {
        return NULL;
    }
    send_interrupcion(fd_cpu_interrupt, pcb->pid);
    return NULL;
}

void controlar_quantum(t_pcb* pcb) {
    pthread_create(&quantum_thread, NULL, (void *)quantum, (void *)pcb);
    pthread_detach(quantum_thread);
}

int size_all_queues() {
    // Ya esta creada de antes colaAux por lo que si no es VRR sera 0
    int size_queues = 0;

    pthread_mutex_lock(&colaReadyMutex);
    size_queues += queue_size(colaReady);
    pthread_mutex_unlock(&colaReadyMutex);

    pthread_mutex_lock(&colaExecMutex);
    size_queues += queue_size(colaExec);
    pthread_mutex_unlock(&colaExecMutex);

    pthread_mutex_lock(&colaBlockedMutex);
    size_queues += queue_size(colaBlocked);
    pthread_mutex_unlock(&colaBlockedMutex);

    pthread_mutex_lock(&colaAuxMutex);
    size_queues += queue_size(colaAux);
    pthread_mutex_unlock(&colaAuxMutex);

    for(int i = 0; i < list_size(recursos); i++) {
        t_recurso* recurso = list_get(recursos, i);
        if(!queue_is_empty(recurso->blocked)) {
            size_queues += queue_size(recurso->blocked);
        }
    }
    return size_queues;
}

void verificar_si_todos_estan_bloqueados() {
    int total_size = 0;

    pthread_mutex_lock(&colaNewMutex);
    total_size += queue_size(colaNew);
    pthread_mutex_unlock(&colaNewMutex);

    pthread_mutex_lock(&colaExecMutex);
    total_size += queue_size(colaExec);
    pthread_mutex_unlock(&colaExecMutex);

    pthread_mutex_lock(&colaAuxMutex);
    total_size += queue_size(colaAux);
    pthread_mutex_unlock(&colaAuxMutex);

    pthread_mutex_lock(&colaReadyMutex);
    total_size += queue_size(colaReady);
    pthread_mutex_unlock(&colaReadyMutex);
    
    if (total_size == 0) {
        sem_wait(&semaforoPlanificacion2);
    }
}

// Log minimo de ingreso a Ready / Ready Prioridad
void ingreso_ready_aux(t_queue* cola, pthread_mutex_t mutex, char* estado) {
    pthread_mutex_lock(&mutex);
    if(queue_is_empty(cola)) {  
    	log_info(kernel_logger, "Cola %s: []", estado);
    } else {
		char lista_pids[128] = "";
        t_list* elements = cola->elements;
        for(int i = 0; i < list_size(elements); i++) {
            t_pcb* pcb = (t_pcb*) list_get(elements, i);
			char pid_str[12];  // Para convertir pid a string
            snprintf(pid_str, sizeof(pid_str), "%d, ", pcb->pid);
            strcat(lista_pids, pid_str);
        }
		lista_pids[strlen(lista_pids) - 2] = '\0';
    	log_info(kernel_logger, "Cola %s: [%s]", estado, lista_pids);
    }
    pthread_mutex_unlock(&mutex);
}

void planificacionFIFO() {
    // El control_planificacion se cambia a 0 una vez que se elimina el proceso y todas las queues estan vacias
    // Esto es por si solamente hay un proceso ejecutando y se interrumpe para que no haya problemas
    control_planificacion = 1;
    while (control_planificacion) {
        sem_wait(&semaforoPlanificacion);
        sem_wait(&semaforoPlanificacion2);
        
        while (queue_size(colaNew) > 0 && contador_procesos < GRADO_MULTIPROGRAMACION) {
            pthread_mutex_lock(&colaNewMutex);
            t_pcb* pcb_nuevo = queue_pop(colaNew);
            pthread_mutex_unlock(&colaNewMutex);
            pcb_nuevo->estado = 'R';
            pthread_mutex_lock(&colaReadyMutex);
            queue_push(colaReady, pcb_nuevo);
            pthread_mutex_unlock(&colaReadyMutex);
            log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_nuevo->pid, "New", "Ready");
            ingreso_ready_aux(colaReady, colaReadyMutex, "Ready");
            contador_procesos++;
        }

        if (queue_size(colaReady) > 0 && queue_size(colaExec) == 0) {
            pthread_mutex_lock(&colaExecMutex);
            pthread_mutex_lock(&colaReadyMutex);
            t_pcb* pcb = queue_pop(colaReady);
            pthread_mutex_unlock(&colaReadyMutex);
            pcb->estado = 'E';
            queue_push(colaExec, pcb);
            pthread_mutex_unlock(&colaExecMutex);

            // Manda PID del proceso a ejecutar
            send_pid(fd_cpu_dispatch, pcb->pid);
            log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, "Ready", "Exec");
        }
        sem_post(&semaforoPlanificacion2);
        sem_post(&semaforoPlanificacion);

        // Para que la consola no tenga lag cuando todos los procesos estan bloqueados
        // Ademas cuando termina hace sem_wait de la planificacion para que se pueda iniciar otra
        verificar_si_todos_estan_bloqueados();

        if (queue_size(colaExec) != 0) {
            sem_wait(&semaforoPlanificacion);
        }
    }

    // sem_wait(&semaforoPlanificacion);
    control_primera_vez = true;
    queue_clean_and_destroy_elements(colaNew, destruir_pcb);
    queue_clean_and_destroy_elements(colaReady, destruir_pcb);
    queue_clean_and_destroy_elements(colaExec, destruir_pcb);
    queue_clean_and_destroy_elements(colaBlocked, destruir_pcb);
    queue_clean_and_destroy_elements(colaAux,destruir_pcb);
}

void planificacionRR() {
    // El control_planificacion se cambia a 0 una vez que se elimina el proceso y todas las queues estan vacias
    // Esto es por si solamente hay un proceso ejecutando y se interrumpe para que no haya problemas
    control_planificacion = 1;
    while (control_planificacion) {
        sem_wait(&semaforoPlanificacion);
        sem_wait(&semaforoPlanificacion2);

        while (queue_size(colaNew) > 0 && contador_procesos < GRADO_MULTIPROGRAMACION) {
            pthread_mutex_lock(&colaNewMutex);
            t_pcb* pcb_nuevo = queue_pop(colaNew);
            pthread_mutex_unlock(&colaNewMutex);
            pcb_nuevo->estado = 'R';
            pthread_mutex_lock(&colaReadyMutex);
            queue_push(colaReady, pcb_nuevo);
            pthread_mutex_unlock(&colaReadyMutex);
            log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_nuevo->pid, "New", "Ready");
            ingreso_ready_aux(colaReady, colaReadyMutex, "Ready");
            contador_procesos++;
        }

        if (queue_size(colaReady) > 0 && queue_size(colaExec) == 0) {
            pthread_mutex_lock(&colaExecMutex);
            pthread_mutex_lock(&colaReadyMutex);
            t_pcb* pcb = queue_pop(colaReady);
            pthread_mutex_unlock(&colaReadyMutex);
            pcb->estado = 'E';
            queue_push(colaExec, pcb);
            pthread_mutex_unlock(&colaExecMutex);

            // Manda PID del proceso a ejecutar
            send_pid(fd_cpu_dispatch, pcb->pid);
            log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, "Ready", "Exec");
            
            // Quantum
            controlar_quantum(pcb);
        }
        sem_post(&semaforoPlanificacion);
        sem_post(&semaforoPlanificacion2);

        // Para que la consola no tenga lag cuando todos los procesos estan bloqueados
        // Ademas cuando termina hace sem_wait de la planificacion para que se pueda iniciar otra
        verificar_si_todos_estan_bloqueados();

        if (queue_size(colaExec) != 0) {
            sem_wait(&semaforoPlanificacion);
        }
    }

    // sem_wait(&semaforoPlanificacion);
    control_primera_vez = true;
    queue_clean_and_destroy_elements(colaNew, destruir_pcb);
    queue_clean_and_destroy_elements(colaReady, destruir_pcb);
    queue_clean_and_destroy_elements(colaExec, destruir_pcb);
    queue_clean_and_destroy_elements(colaBlocked, destruir_pcb);
    queue_clean_and_destroy_elements(colaAux, destruir_pcb);
}

void planificacionVRR() {
    // El control_planificacion se cambia a 0 una vez que se elimina el proceso y todas las queues estan vacias
    // Esto es por si solamente hay un proceso ejecutando y se interrumpe para que no haya problemas
    control_planificacion = 1;
    while (control_planificacion) {
        sem_wait(&semaforoPlanificacion);
        sem_wait(&semaforoPlanificacion2);

        while (queue_size(colaNew) > 0 && contador_procesos < GRADO_MULTIPROGRAMACION) {
            pthread_mutex_lock(&colaNewMutex);
            t_pcb* pcb_nuevo = queue_pop(colaNew);
            pthread_mutex_unlock(&colaNewMutex);
            pcb_nuevo->estado = 'R';
            pthread_mutex_lock(&colaReadyMutex);
            queue_push(colaReady, pcb_nuevo);
            pthread_mutex_unlock(&colaReadyMutex);
            log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_nuevo->pid, "New", "Ready");
            ingreso_ready_aux(colaReady, colaReadyMutex, "Ready");
            contador_procesos++;
        }

        if(queue_size(colaAux) > 0 && queue_size(colaExec) == 0) {
            //Ejecutar los procesos de esta cola
            t_pcb* pcb;

            // Ejecutamos el proceso de la cola
            pthread_mutex_lock(&colaExecMutex);
            pthread_mutex_lock(&colaAuxMutex);
            pcb = queue_pop(colaAux);
            pthread_mutex_unlock(&colaAuxMutex);
            pcb->estado = 'E';
            queue_push(colaExec, pcb);
            pthread_mutex_unlock(&colaExecMutex);

            // Manda PID del proceso a ejecutar
            send_pid(fd_cpu_dispatch, pcb->pid);
            // Quantum
            controlar_quantum(pcb);
            // Aca empeza a contar el tiempo de ejecucion --> crear variable de tiempo
            tiempo_vrr = temporal_create();
            log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, "Ready (Prioritario)", "Exec");
        }
        
        if (queue_size(colaReady) > 0 && queue_size(colaExec) == 0 && queue_size(colaAux) == 0) {
            pthread_mutex_lock(&colaExecMutex);
            pthread_mutex_lock(&colaReadyMutex);
            t_pcb* pcb = queue_pop(colaReady);
            pthread_mutex_unlock(&colaReadyMutex);
            pcb->estado = 'E';
            queue_push(colaExec, pcb);
            pthread_mutex_unlock(&colaExecMutex);

            // Manda PID del proceso a ejecutar
            send_pid(fd_cpu_dispatch, pcb->pid);
            // Quantum
            controlar_quantum(pcb);
            // Aca empeza a contar el tiempo de ejecucion --> crear variable de tiempo
            tiempo_vrr = temporal_create();
            log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, "Ready", "Exec");
        }
        sem_post(&semaforoPlanificacion2);
        sem_post(&semaforoPlanificacion);

        // Para que la consola no tenga lag cuando todos los procesos estan bloqueados
        // Ademas cuando termina hace sem_wait de la planificacion para que se pueda iniciar otra
        verificar_si_todos_estan_bloqueados();
    }

    // sem_wait(&semaforoPlanificacion);
    control_primera_vez = true;
    queue_clean_and_destroy_elements(colaNew, destruir_pcb);
    queue_clean_and_destroy_elements(colaReady, destruir_pcb);
    queue_clean_and_destroy_elements(colaExec, destruir_pcb);
    queue_clean_and_destroy_elements(colaBlocked, destruir_pcb);
    queue_clean_and_destroy_elements(colaAux, destruir_pcb);
}