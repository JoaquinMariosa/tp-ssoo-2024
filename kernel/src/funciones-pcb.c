#include "../include/funciones-pcb.h"

uint8_t contador_pid = 0;

void asignar_pid(t_pcb* pcb) {
    // Wait mutex de semaforo
    
    pcb->pid = contador_pid;
    contador_pid++;

    // Signal mutex de semaforo
}

t_pcb* crear_pcb() {
    // Asignar memoria para el PCB
    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    if (nuevo_pcb == NULL) {
        printf("Error al asignar memoria para el PCB\n");
        return NULL;
    }

    // Asignar valores iniciales
    asignar_pid(nuevo_pcb);         // Asignar un PID único.
    nuevo_pcb->pc = 0;              // Inicialmente, el Program Counter está en 0.
    nuevo_pcb->estado = 'N';        // El estado inicial es 'Nuevo'.
    nuevo_pcb->quantum = QUANTUM;   // Valor predeterminado para el quantum.
    nuevo_pcb->flag_int = 0;        // Valor predeterminado en el cual no hubo interrupcion

    // Asignar e inicializar los registros
    nuevo_pcb->registros = malloc(sizeof(t_registros));
    if (nuevo_pcb->registros == NULL) {
        printf("Error al asignar memoria para los registros del PCB\n");
        free(nuevo_pcb);
        return NULL;
    }

    // Inicializar los registros a valores predeterminados (todos en cero)
    nuevo_pcb->registros->AX  = 0;
    nuevo_pcb->registros->BX  = 0;
    nuevo_pcb->registros->CX  = 0;
    nuevo_pcb->registros->DX  = 0;
    nuevo_pcb->registros->EAX = 0;
    nuevo_pcb->registros->EBX = 0;
    nuevo_pcb->registros->ECX = 0;
    nuevo_pcb->registros->EDX = 0;
    nuevo_pcb->registros->SI  = 0;
    nuevo_pcb->registros->DI  = 0;

    return nuevo_pcb;
}